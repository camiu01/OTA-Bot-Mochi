/**
 * @file ui_theme.cpp
 * @brief Compact, high-contrast drawing helpers for the 240x280 canvas.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ui_theme.h"
#include "../../pin_config.h"
#include "../power/backlight.h"

uint16_t UI_CARD = 0x18C3;
uint16_t UI_TEXT = 0xFFFF;
uint16_t UI_MUTED = 0xA514;
uint16_t UI_ACCENT = 0xFFFF;
uint16_t UI_WARM = 0xFFFF;

/** @brief Scale RGB565 components independently, preserving their hue. */
static uint16_t shade(uint16_t color, uint8_t percent) {
	return (((color >> 11) * percent / 100) << 11) |
		((((color >> 5) & 63) * percent / 100) << 5) |
		((color & 31) * percent / 100);
}

void ui_theme_color(uint16_t color) {
	UI_TEXT = UI_ACCENT = UI_WARM = color;
	UI_MUTED = shade(color, 65);
	UI_CARD = shade(color, 10);
}

/**
 * @brief Convert text to safe ASCII, approximating common Latin accents.
 * @param text UTF-8 source.
 * @return ASCII text with unsupported glyphs represented by a question mark.
 */
String ui_ascii(const String &text) {
	String out;
	out.reserve(text.length());
	for (unsigned int i = 0; i < text.length(); ++i) {
		uint8_t c = text[i];
		if (c < 128) {
			out += (c == '\n' || c >= 32) ? char(c) : ' ';
			continue;
		}
		char replacement = '?';
		if (c == 0xC3 && i + 1 < text.length()) {
			uint8_t next = text[i + 1];
			if (next >= 0xA0 && next <= 0xA5) { replacement = 'a'; }
			if (next >= 0xA8 && next <= 0xAB) { replacement = 'e'; }
			if (next >= 0xAC && next <= 0xAF) { replacement = 'i'; }
			if (next >= 0xB2 && next <= 0xB6) { replacement = 'o'; }
			if (next >= 0xB9 && next <= 0xBC) { replacement = 'u'; }
		}
		out += replacement;
		while (i + 1 < text.length() && (uint8_t(text[i + 1]) & 0xC0) == 0x80) {
			++i;
		}
	}
	return out;
}

/**
 * @brief Draw a single ASCII line with ellipsis clipping.
 * @param gfx Canvas.
 * @param text Source.
 * @param x Left edge.
 * @param y Top edge.
 * @param width Available width.
 * @param size Font scale.
 * @param color Foreground.
 */
void ui_label(Arduino_GFX *gfx, const String &text, int16_t x, int16_t y,
	int16_t width, uint8_t size, uint16_t color) {
	if (size == 0 || width < 6 * size) { return; }
	String safe = ui_ascii(text);
	safe.replace('\n', ' ');
	unsigned int columns = width / (6 * size);
	if (safe.length() > columns) {
		safe = safe.substring(0, columns > 3 ? columns - 3 : 0);
		for (unsigned int i = 0; i < (columns < 3 ? columns : 3); ++i) { safe += '.'; }
	}
	gfx->setFont(nullptr);
	gfx->setTextWrap(false);
	gfx->setTextSize(size);
	gfx->setTextColor(color);
	gfx->setCursor(x, y);
	gfx->print(safe);
}

/** @brief Draw a content card. @param gfx Canvas. @param y Top. @param height Height. */
void ui_card(Arduino_GFX *gfx, int16_t y, int16_t height) {
	gfx->fillRoundRect(12, y, LCD_WIDTH - 24, height, 14, UI_CARD);
}

/**
 * @brief Render header and pagination with the selected eye-color accent.
 * @param gfx Canvas.
 * @param title Screen title.
 * @param connected Bluetooth flag.
 * @param page Screen index.
 */
void ui_chrome(Arduino_GFX *gfx, const char *title, bool connected, uint8_t page) {
	gfx->fillRect(0, 0, LCD_WIDTH, 52, UI_BG);
	ui_label(gfx, "MOCHI / COMPANION", 16, 12, 180, 1, UI_MUTED);
	ui_label(gfx, String(backlight_brightness()) + "%", 182, 12, 30, 1, UI_MUTED);
	gfx->fillCircle(220, 16, 3, connected ? UI_ACCENT : UI_MUTED);
	ui_label(gfx, title, 16, 30, 198, 2, UI_TEXT);
	gfx->fillRect(0, 258, LCD_WIDTH, 22, UI_BG);
	for (uint8_t i = 0; i < 6; ++i) {
		gfx->fillCircle(90 + i * 12, 267, i == page ? 3 : 2,
			i == page ? UI_ACCENT : UI_MUTED);
	}
}

/**
 * @brief Draw an empty state that explains how data arrives.
 * @param gfx Canvas.
 * @param title Primary message.
 * @param hint Next action.
 */
void ui_empty(Arduino_GFX *gfx, const char *title, const char *hint) {
	ui_card(gfx, 66, 172);
	gfx->drawRoundRect(96, 92, 48, 38, 12, UI_ACCENT);
	gfx->fillCircle(109, 110, 3, UI_TEXT);
	gfx->fillCircle(131, 110, 3, UI_TEXT);
	ui_label(gfx, title, 28, 154, 184, 1, UI_TEXT);
	ui_label(gfx, hint, 28, 181, 184, 1, UI_MUTED);
}

/**
 * @brief Draw sun, clouds, rain, snow, or wind using bounded primitives.
 * @param gfx Canvas.
 * @param code Chronos condition.
 * @param x Center X.
 * @param y Center Y.
 */
void ui_weather_icon(Arduino_GFX *gfx, int code, int16_t x, int16_t y) {
	if (code < 0 || code > 7) {
		gfx->drawCircle(x, y, 18, UI_MUTED);
		ui_label(gfx, "?", x - 5, y - 8, 12, 2, UI_MUTED);
		return;
	}
	if (code == 0 || code == 1) {
		gfx->fillCircle(x, y - 5, 12, UI_WARM);
		for (int8_t dx : {-20, 20}) { gfx->drawFastHLine(x + dx - 3, y - 5, 6, UI_WARM); }
		gfx->drawFastVLine(x, y - 27, 6, UI_WARM);
	}
	if (code == 1) { return; }
	if (code == 5 || code == 6 || code == 7) {
		for (int8_t i = 0; i < 3; ++i) {
			gfx->drawFastHLine(x - 22 + i * 3, y - 12 + i * 10, 40 - i * 4, UI_MUTED);
		}
		return;
	}
	gfx->fillCircle(x - 10, y, 10, UI_TEXT);
	gfx->fillCircle(x + 2, y - 7, 14, UI_TEXT);
	gfx->fillRoundRect(x - 20, y, 42, 12, 6, UI_TEXT);
	if (code == 2 || code == 3) {
		for (int8_t i = -1; i <= 1; ++i) {
			if (code == 2) { gfx->fillCircle(x + i * 12, y + 22, 2, UI_ACCENT); }
			else { gfx->drawLine(x + i * 12, y + 17, x + i * 12 - 3, y + 24, UI_ACCENT); }
		}
	}
}

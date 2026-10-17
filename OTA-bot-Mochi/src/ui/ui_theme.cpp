/**
 * @file ui_theme.cpp
 * @brief Compact, high-contrast drawing helpers for the LCD_WIDTH x LCD_HEIGHT canvas.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ui_theme.h"
#include "../../pin_config.h"
#include "../power/backlight.h"
#include "../sound/sound.h"

uint16_t UI_CARD = 0x18C3;
uint16_t UI_TEXT = 0xFFFF;
uint16_t UI_MUTED = 0xA514;
uint16_t UI_ACCENT = 0xFFFF;
uint16_t UI_WARM = 0xFFFF;

int16_t ui_scale() {
	int16_t sx = int16_t(LCD_WIDTH) * 100 / UI_DESIGN_W;
	int16_t sy = int16_t(LCD_HEIGHT) * 100 / UI_DESIGN_H;
	int16_t base = sx < sy ? sx : sy;
	return base < 100 ? 100 : (base > 200 ? 200 : base);
}

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
	size = ui_text_size(size);
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

/** @brief Draw a content card. @param gfx Canvas. @param y Top (design space). @param height Height (design space). */
void ui_card(Arduino_GFX *gfx, int16_t y, int16_t height) {
	gfx->fillRoundRect(ui_s(12), ui_s(y), LCD_WIDTH - 2 * ui_s(12), ui_s(height), ui_s(14), UI_CARD);
}

/**
 * @brief Draw the compact speaker glyph, crossed out whenever audio cannot sound.
 * @param gfx Canvas.
 * @param x Left edge.
 * @param y Top edge.
 */
void ui_sound_badge(Arduino_GFX *gfx, int16_t x, int16_t y) {
	SoundStatus audio = sound_status();
	bool audible = audio == SoundStatus::Armed || audio == SoundStatus::Ready;
	uint16_t color = audible ? UI_ACCENT : UI_MUTED;
	gfx->fillRect(x, y + 3, 4, 7, color);
	gfx->fillTriangle(x + 4, y, x + 4, y + 12, x + 11, y + 6, color);
	if (audible) {
		gfx->drawLine(x + 14, y + 2, x + 14, y + 10, color);
		gfx->drawLine(x + 17, y + 4, x + 17, y + 8, color);
	} else {
		gfx->drawLine(x + 14, y + 2, x + 20, y + 10, color);
		gfx->drawLine(x + 20, y + 2, x + 14, y + 10, color);
	}
}

void ui_sound_badge_clear(Arduino_GFX *gfx, int16_t x, int16_t y) {
	gfx->fillRect(x, y, 22, 13, UI_BG);
}

/**
 * @brief Render header and pagination with the selected eye-color accent.
 * @param gfx Canvas.
 * @param title Screen title.
 * @param connected Bluetooth flag.
 * @param page Screen index.
 */
void ui_chrome(Arduino_GFX *gfx, const char *title, bool connected, uint8_t page) {
	gfx->fillRect(0, 0, LCD_WIDTH, ui_s(52), UI_BG);
	ui_label(gfx, "MOCHI / COMPANION", ui_s(16), ui_s(12), ui_s(180), 1, UI_MUTED);
	ui_label(gfx, String(backlight_brightness()) + "%", ui_s(182), ui_s(12), ui_s(34), 1, UI_MUTED);
	gfx->fillCircle(ui_s(220), ui_s(16), 3, connected ? UI_ACCENT : UI_MUTED);
	// Doubled header text ends later on scaled panels, so the badge steps aside instead of colliding.
	int16_t badge_x = ui_s(150);
	int16_t title_right = int16_t(ui_s(16) + 17 * 6 * ui_text_size(1));
	if (badge_x < title_right + 8) { badge_x = int16_t(title_right + 8); }
	ui_sound_badge(gfx, badge_x, ui_s(11));
	ui_label(gfx, title, ui_s(16), ui_s(30), ui_s(208), 2, UI_TEXT);
	gfx->fillRect(0, ui_s(258), LCD_WIDTH, ui_s(22), UI_BG);
	for (uint8_t i = 0; i < 6; ++i) {
		gfx->fillCircle(ui_s(90) + i * ui_s(12), ui_s(267), i == page ? 3 : 2,
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
	gfx->drawRoundRect(ui_s(96), ui_s(92), ui_s(48), ui_s(38), ui_s(12), UI_ACCENT);
	gfx->fillCircle(ui_s(109), ui_s(110), 3, UI_TEXT);
	gfx->fillCircle(ui_s(131), ui_s(110), 3, UI_TEXT);
	ui_label(gfx, title, ui_s(28), ui_s(154), ui_s(184), 1, UI_TEXT);
	ui_label(gfx, hint, ui_s(28), ui_s(181), ui_s(184), 1, UI_MUTED);
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
		gfx->drawCircle(x, y, ui_s(18), UI_MUTED);
		ui_label(gfx, "?", x - ui_s(5), y - ui_s(8), ui_s(12), 2, UI_MUTED);
		return;
	}
	if (code == 0 || code == 1) {
		gfx->fillCircle(x, y - ui_s(5), ui_s(12), UI_WARM);
		for (int8_t dx : {-20, 20}) { gfx->drawFastHLine(x + ui_s(dx) - ui_s(3), y - ui_s(5), ui_s(6), UI_WARM); }
		gfx->drawFastVLine(x, y - ui_s(27), ui_s(6), UI_WARM);
	}
	if (code == 1) { return; }
	if (code == 5 || code == 6 || code == 7) {
		for (int8_t i = 0; i < 3; ++i) {
			gfx->drawFastHLine(x - ui_s(22) + i * ui_s(3), y - ui_s(12) + i * ui_s(10), ui_s(40 - i * 4), UI_MUTED);
		}
		return;
	}
	gfx->fillCircle(x - ui_s(10), y, ui_s(10), UI_TEXT);
	gfx->fillCircle(x + ui_s(2), y - ui_s(7), ui_s(14), UI_TEXT);
	gfx->fillRoundRect(x - ui_s(20), y, ui_s(42), ui_s(12), ui_s(6), UI_TEXT);
	if (code == 2 || code == 3) {
		for (int8_t i = -1; i <= 1; ++i) {
			if (code == 2) { gfx->fillCircle(x + i * ui_s(12), y + ui_s(22), 2, UI_ACCENT); }
			else { gfx->drawLine(x + i * ui_s(12), y + ui_s(17), x + i * ui_s(12) - ui_s(3), y + ui_s(24), UI_ACCENT); }
		}
	}
}

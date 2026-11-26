/**
 * @file ui_theme.cpp
 * @brief Compact, high-contrast drawing helpers for the LCD_WIDTH x LCD_HEIGHT canvas.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ui_theme.h"
#include "ui_font.h"
#include "../../pin_config.h"
#include "../power/backlight.h"
#include "../power/battery.h"
#include "../power/charging.h"
#include "../sound/sound.h"

uint16_t UI_CARD = 0x1082;
uint16_t UI_LINE = 0x2945;
uint16_t UI_TEXT = 0xEF5D;
uint16_t UI_MUTED = 0x8C51;
uint16_t UI_ACCENT = 0xFFFF;
uint16_t UI_WARM = 0xFD20;

int16_t ui_scale() {
	int16_t sx = int16_t(LCD_WIDTH) * 100 / UI_DESIGN_W;
	int16_t sy = int16_t(LCD_HEIGHT) * 100 / UI_DESIGN_H;
	int16_t base = sx < sy ? sx : sy;
	return base < 100 ? 100 : (base > 200 ? 200 : base);
}

/**
 * @brief Minimal dark palette: neutral ink and surfaces, the eye color only as accent.
 *
 * Text stays near-white and secondary text neutral grey on every eye color, so
 * contrast no longer depends on the selected hue. Surfaces are neutral grey;
 * warnings use a fixed amber.
 * @param color Selected eye color (RGB565).
 */
void ui_theme_color(uint16_t color) {
	UI_ACCENT = color;
	UI_TEXT = 0xEF5D;	// RGB565 greys keep green at exactly twice red/blue: no green cast.
	UI_MUTED = 0x8C51;
	UI_WARM = color == 0xFD6C ? 0xF9D6 : 0xFD20;	// Amber eyes warn in pink.
	UI_CARD = 0x1082;	// Neutral surfaces: no accent tint, so no hue cast on large areas.
	UI_LINE = 0x2945;
}

void ui_box(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t fill, bool outline) {
	int16_t r = ui_s(12) < h / 2 ? ui_s(12) : h / 2;
	gfx->fillRoundRect(x, y, w, h, r, fill);
	if (outline) { gfx->drawRoundRect(x, y, w, h, r, UI_LINE); }
}

void ui_button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, const String &label, bool primary, bool enabled) {
	const uint16_t fill = primary && enabled ? UI_ACCENT : UI_CARD;
	ui_box(gfx, x, y, w, h, fill, !primary || !enabled);
	const int16_t room = w - ui_s(12);
	const String text = ui_font::fit(ui_ascii(label), room, 1);
	const int16_t width = ui_font::width(text, 1);
	ui_font::label(gfx, text, x + (w - width) / 2, y + (h - ui_font::height(1)) / 2, room, 1,
		!enabled ? UI_MUTED : (primary ? UI_BG : UI_TEXT), fill);
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
	if (ui_font::draw(gfx, text, x, y, width, size, color)) { return; }
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

void ui_center(Arduino_GFX *gfx, const String &text, int16_t y, uint8_t size, uint16_t color) {
	const int16_t room = LCD_WIDTH - ui_s(48);
	const String clipped = ui_font::fit(ui_ascii(text), room, size);
	const int16_t width = ui_font::width(clipped, size);
	ui_label(gfx, clipped, (LCD_WIDTH - width) / 2, ui_s(y), room, size, color);
}

void ui_bar(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int32_t permille, uint16_t color) {
	permille = permille < 0 ? 0 : (permille > 1000 ? 1000 : permille);
	gfx->fillRoundRect(x, y, w, h, h / 2, UI_LINE);
	int16_t filled = int16_t(int32_t(w) * permille / 1000);
	if (filled >= h) { gfx->fillRoundRect(x, y, filled, h, h / 2, color); }
}

void ui_status(Arduino_GFX *gfx, int16_t y, bool good, const String &text) {
	int16_t r = ui_s(3) < 2 ? 2 : ui_s(3);
	gfx->fillCircle(ui_s(32), ui_s(y) + 7 * ui_text_size(1) / 2, r, good ? UI_ACCENT : UI_LINE);
	ui_label(gfx, text, ui_s(44), ui_s(y), ui_s(168), 1, good ? UI_TEXT : UI_MUTED);
}

/** @brief Draw a content card. @param gfx Canvas. @param y Top (design space). @param height Height (design space). */
void ui_card(Arduino_GFX *gfx, int16_t y, int16_t height) {
	ui_box(gfx, ui_s(12), ui_s(y), LCD_WIDTH - 2 * ui_s(12), ui_s(height), UI_CARD, true);
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
	gfx->fillRect(x, y + 4, 5, 5, color);
	gfx->fillTriangle(x + 4, y + 4, x + 10, y, x + 10, y + 12, color);
	gfx->fillTriangle(x + 4, y + 4, x + 10, y + 12, x + 4, y + 8, color);
	if (audible) {
		gfx->drawLine(x + 14, y + 3, x + 16, y + 5, color);
		gfx->drawLine(x + 16, y + 5, x + 16, y + 7, color);
		gfx->drawLine(x + 16, y + 7, x + 14, y + 9, color);
		gfx->drawLine(x + 17, y, x + 20, y + 3, color);
		gfx->drawLine(x + 20, y + 3, x + 21, y + 6, color);
		gfx->drawLine(x + 21, y + 6, x + 20, y + 9, color);
		gfx->drawLine(x + 20, y + 9, x + 17, y + 12, color);
	} else {
		gfx->drawLine(x + 14, y + 2, x + 20, y + 10, color);
		gfx->drawLine(x + 20, y + 2, x + 14, y + 10, color);
	}
}

/**
 * @brief Draw a battery outline and inset bolt while the PMU reports charging.
 * @param gfx Canvas.
 * @param x Left edge; glyph spans x..x+7 and y..y+12.
 * @param y Top edge.
 */
void ui_charge_badge(Arduino_GFX *gfx, int16_t x, int16_t y) {
	uint16_t color = UI_ACCENT;
	gfx->fillRect(x + 2, y, 4, 2, color);
	gfx->drawRect(x, y + 2, 8, 11, color);
	gfx->fillTriangle(x + 5, y + 4, x + 2, y + 8, x + 4, y + 8, color);
	gfx->fillTriangle(x + 3, y + 11, x + 6, y + 7, x + 4, y + 7, color);
}

/**
 * @brief Render header and pagination with the selected eye-color accent.
 * @param gfx Canvas.
 * @param title Screen title.
 * @param connected Bluetooth flag.
 * @param page Screen index.
 * @param settings_page True when the settings footer occupies the carousel indicator area.
 */
void ui_chrome(Arduino_GFX *gfx, const char *title, bool connected, uint8_t page, bool settings_page) {
	gfx->fillRect(0, 0, LCD_WIDTH, ui_s(52), UI_BG);
	uint8_t charge = battery_percent();
	bool charging = charging_active();
	ui_label(gfx, charge == BATTERY_UNAVAILABLE ? String("--%") : String(charge) + "%",
		ui_s(174), ui_s(12), ui_s(32), 1, charging ? UI_ACCENT : UI_MUTED);
	if (charging) { ui_charge_badge(gfx, ui_s(208), ui_s(11)); }
	gfx->fillCircle(ui_s(220), ui_s(16), 3, connected ? UI_ACCENT : UI_MUTED);
	ui_sound_badge(gfx, ui_s(150), ui_s(11));
	gfx->fillRoundRect(ui_s(6), ui_s(30), ui_s(4), ui_s(16), ui_s(2), UI_ACCENT);
	ui_label(gfx, title, ui_s(16), ui_s(30), ui_s(208), 2, UI_TEXT);
	gfx->fillRect(0, ui_s(258), LCD_WIDTH, ui_s(22), UI_BG);
	if (settings_page && ui_font::Supported) { return; }
	// Pagination: a short accent pill marks the current page between small dots.
	const int16_t step = ui_s(12), pill = ui_s(16), dot = ui_s(2) < 2 ? 2 : ui_s(2);
	int16_t x = int16_t((LCD_WIDTH - (7 * step + pill)) / 2), y = ui_s(267);
	for (uint8_t i = 0; i < 8; ++i) {
		if (i == page) {
			gfx->fillRoundRect(x, y - dot - 1, pill, 2 * dot + 3, dot + 1, UI_ACCENT);
			x += pill + step - 2 * dot;
		} else {
			gfx->fillCircle(x + dot, y, dot, UI_MUTED);
			x += step;
		}
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

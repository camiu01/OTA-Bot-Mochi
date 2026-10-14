/**
 * @file Arduino_GFX_Library.h
 * @brief Native mock recording paint calls, checking text bounds, and rasterizing text.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "Arduino.h"
#include <vector>
#include <assert.h>
#include "raster.h"
#include "bitmap_font.h"
#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF

class Arduino_GFX : public Raster {
	int16_t cursor_x = 0;
	int16_t cursor_y = 0;
	uint8_t size = 1;
	uint16_t text_color = RGB565_WHITE;

	/** @brief Rasterize one character at the cursor, then advance it. @param ch Character. */
	void draw_char(char ch) {
		unsigned code = static_cast<unsigned char>(ch);
		if (code >= 128) { code = '?'; }
		const uint8_t *glyph = bitmap_font5x7 + code * 5;
		for (int col = 0; col < 5; ++col) {
			uint8_t bits = glyph[col];
			for (int row = 0; row < 8; ++row, bits >>= 1) {
				if (!(bits & 1)) { continue; }
				for (int dx = 0; dx < size; ++dx) {
					for (int dy = 0; dy < size; ++dy) {
						pixel(cursor_x + col * size + dx, cursor_y + row * size + dy, text_color);
					}
				}
			}
		}
		cursor_x += 6 * size;
	}
public:
	unsigned int paints = 0;
	std::vector<std::string> text;
	void fillRect(int x, int y, int w, int h, uint16_t color) { rectangle(x, y, w, h, color); ++paints; }
	void drawRect(int x, int y, int w, int h, uint16_t color) {
		fillRect(x, y, w, 1, color);
		fillRect(x, y + h - 1, w, 1, color);
		fillRect(x, y, 1, h, color);
		fillRect(x + w - 1, y, 1, h, color);
	}
	void fillRoundRect(int x, int y, int w, int h, int r, uint16_t color) { rounded(x, y, w, h, r, color); ++paints; }
	template<class... T> void drawRoundRect(T...) { ++paints; }
	void fillCircle(int x, int y, int r, uint16_t color) { ellipse(x, y, r, r, color); ++paints; }
	void fillEllipse(int x, int y, int rx, int ry, uint16_t color) { ellipse(x, y, rx, ry, color); ++paints; }
	void fillTriangle(int ax, int ay, int bx, int by, int cx, int cy, uint16_t color) {
		triangle(ax, ay, bx, by, cx, cy, color); ++paints;
	}
	template<class... T> void drawCircle(T...) { ++paints; }
	void drawLine(int ax, int ay, int bx, int by, uint16_t color) { line(ax, ay, bx, by, color); ++paints; }
	template<class... T> void drawFastHLine(T...) { ++paints; }
	template<class... T> void drawFastVLine(T...) { ++paints; }
	template<class... T> void drawBitmap(T...) { ++paints; }
	void fillScreen(uint16_t color) { std::fill(pixels.begin(), pixels.end(), color); ++paints; }
	void setFont(const void *) {}
	void setTextSize(uint8_t scale) { size = scale; }
	void setTextColor(uint16_t color) { text_color = color; }
	void setTextWrap(bool) {}
	void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
	void print(const String &line) {
		assert(cursor_x >= 0 && cursor_x + int(line.length()) * 6 * size <= 240);
		assert(cursor_y >= 0 && cursor_y + 8 * size <= 280);
		text.emplace_back(line.c_str());
		++paints;
		for (size_t i = 0; i < line.length(); ++i) { draw_char(line[i]); }
	}
};

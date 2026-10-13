/**
 * @file Arduino_GFX_Library.h
 * @brief Native mock recording paint calls and checking text bounds.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "Arduino.h"
#include <vector>
#include <assert.h>
#include "raster.h"
#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF

class Arduino_GFX : public Raster {
	int16_t cursor_x = 0;
	int16_t cursor_y = 0;
	uint8_t size = 1;
public:
	unsigned int paints = 0;
	std::vector<std::string> text;
	void fillRect(int x, int y, int w, int h, uint16_t color) { rectangle(x, y, w, h, color); ++paints; }
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
	void setTextColor(uint16_t) {}
	void setTextWrap(bool) {}
	void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
	void print(const String &line) {
		assert(cursor_x >= 0 && cursor_x + int(line.length()) * 6 * size <= 240);
		assert(cursor_y >= 0 && cursor_y + 8 * size <= 280);
		text.emplace_back(line.c_str());
		++paints;
	}
};

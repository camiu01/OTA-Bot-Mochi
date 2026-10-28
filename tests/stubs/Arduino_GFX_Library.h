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
	/** @brief Native canvas has no physical display to flush. */
	void flush() {}
	unsigned int paints = 0;
	std::vector<std::string> text;
	/** @brief Allocate a panel-sized canvas. @param panel_w Width in pixels. @param panel_h Height in pixels. */
	explicit Arduino_GFX(int16_t panel_w = LCD_WIDTH, int16_t panel_h = LCD_HEIGHT)
		: Raster(panel_w, panel_h) {}
	void fillRect(int x, int y, int w, int h, uint16_t color) { rectangle(x, y, w, h, color); ++paints; }
	void drawRect(int x, int y, int w, int h, uint16_t color) {
		fillRect(x, y, w, 1, color);
		fillRect(x, y + h - 1, w, 1, color);
		fillRect(x, y, 1, h, color);
		fillRect(x + w - 1, y, 1, h, color);
	}
	void fillRoundRect(int x, int y, int w, int h, int r, uint16_t color) { rounded(x, y, w, h, r, color); ++paints; }
	/** @brief Outline a rounded rectangle: straight edges plus one corner band per quadrant. */
	void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
		if (w <= 0 || h <= 0) { ++paints; return; }
		int m = w < h ? w : h;
		if (r < 0) { r = 0; }
		if (2 * r > m) { r = m / 2; }
		if (w - 2 * r > 0) {
			fillRect(x + r, y, w - 2 * r, 1, color);
			fillRect(x + r, y + h - 1, w - 2 * r, 1, color);
		}
		if (h - 2 * r > 0) {
			fillRect(x, y + r, 1, h - 2 * r, color);
			fillRect(x + w - 1, y + r, 1, h - 2 * r, color);
		}
		for (uint8_t quadrant = 0; quadrant < 4; ++quadrant) {
			int out_x = (quadrant & 1) ? 1 : -1;
			int out_y = (quadrant & 2) ? 1 : -1;
			int cx = x + ((quadrant & 1) ? w - 1 - r : r);
			int cy = y + ((quadrant & 2) ? h - 1 - r : r);
			for (int dy = -r; dy <= r; ++dy) {
				if (dy * out_y < 0) { continue; }
				for (int dx = -r; dx <= r; ++dx) {
					if (dx * out_x < 0) { continue; }
					int d2 = dx * dx + dy * dy;
					if (d2 > r * r || d2 < (r - 1) * (r - 1)) { continue; }
					pixel(cx + dx, cy + dy, color);
				}
			}
		}
		++paints;
	}
	void fillCircle(int x, int y, int r, uint16_t color) { ellipse(x, y, r, r, color); ++paints; }
	void fillEllipse(int x, int y, int rx, int ry, uint16_t color) { ellipse(x, y, rx, ry, color); ++paints; }
	void fillTriangle(int ax, int ay, int bx, int by, int cx, int cy, uint16_t color) {
		triangle(ax, ay, bx, by, cx, cy, color); ++paints;
	}
	/** @brief Outline a circle through the radial band just inside the radius. */
	void drawCircle(int x, int y, int r, uint16_t color) {
		if (r < 0) { ++paints; return; }
		if (!r) { pixel(x, y, color); ++paints; return; }
		for (int dy = -r; dy <= r; ++dy) {
			for (int dx = -r; dx <= r; ++dx) {
				int d2 = dx * dx + dy * dy;
				if (d2 > r * r || d2 < (r - 1) * (r - 1)) { continue; }
				pixel(x + dx, y + dy, color);
			}
		}
		++paints;
	}
	void drawLine(int ax, int ay, int bx, int by, uint16_t color) { line(ax, ay, bx, by, color); ++paints; }
	void drawFastHLine(int x, int y, int len, uint16_t color) {
		if (len > 0) { fillRect(x, y, len, 1, color); }
		++paints;
	}
	void drawFastVLine(int x, int y, int len, uint16_t color) {
		if (len > 0) { fillRect(x, y, 1, len, color); }
		++paints;
	}
	/** @brief Draw a row-major 1-bit bitmap. @param x Left. @param y Top. @param bitmap Bits. @param bw Width. @param bh Height. @param color Foreground. */
	void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t bw, int16_t bh, uint16_t color) {
		int16_t stride = int16_t((bw + 7) / 8);
		for (int16_t row = 0; row < bh; ++row) {
			for (int16_t col = 0; col < bw; ++col) {
				if (bitmap[int(row) * stride + col / 8] & (0x80 >> (col % 8))) { pixel(x + col, y + row, color); }
			}
		}
		++paints;
	}
	void fillScreen(uint16_t color) { std::fill(pixels.begin(), pixels.end(), color); ++paints; }
	void setFont(const void *) {}
	void setTextSize(uint8_t scale) { size = scale; }
	void setTextColor(uint16_t color) { text_color = color; }
	void setTextWrap(bool) {}
	void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
	void print(const String &line) {
		assert(cursor_x >= 0 && cursor_x + int(line.length()) * 6 * size <= w &&
			"text exceeds the panel width");
		assert(cursor_y >= 0 && cursor_y + 8 * size <= h && "text exceeds the panel height");
		text.emplace_back(line.c_str());
		++paints;
		for (size_t i = 0; i < line.length(); ++i) { draw_char(line[i]); }
	}
};

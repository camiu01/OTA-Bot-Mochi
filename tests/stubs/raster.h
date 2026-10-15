/**
 * @file raster.h
 * @brief Native raster substitute for shape/pupil/dirty-region regressions, not LCD proof.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <assert.h>
#include "../../OTA-bot-Mochi/pin_config.h"

struct Raster {
	int16_t w;
	int16_t h;
	std::vector<uint16_t> pixels;
	/** @brief Allocate a panel-sized backing store. @param width Pixels wide. @param height Pixels tall. */
	explicit Raster(int16_t width = LCD_WIDTH, int16_t height = LCD_HEIGHT)
		: w(width), h(height), pixels(size_t(width) * height, 0) {}
	/** @brief Panel width in pixels. @return Width. */
	int16_t width() const { return w; }
	/** @brief Panel height in pixels. @return Height. */
	int16_t height() const { return h; }
	void pixel(int x, int y, uint16_t color) {
		if (x >= 0 && x < w && y >= 0 && y < h) { pixels[size_t(y) * w + x] = color; }
	}
	void rectangle(int x, int y, int rw, int rh, uint16_t color) {
		assert(rw >= 0 && rh >= 0);
		for (int row = std::max(y, 0); row < std::min(y + rh, int(h)); ++row) {
			for (int col = std::max(x, 0); col < std::min(x + rw, int(w)); ++col) { pixel(col, row, color); }
		}
	}
	void ellipse(int x, int y, int rx, int ry, uint16_t color) {
		assert(rx >= 0 && ry >= 0);
		if (!rx || !ry) { pixel(x, y, color); return; }
		for (int row = y - ry; row <= y + ry; ++row) {
			for (int col = x - rx; col <= x + rx; ++col) {
				double dx = double(col - x) / rx, dy = double(row - y) / ry;
				if (dx * dx + dy * dy <= 1) { pixel(col, row, color); }
			}
		}
	}
	void rounded(int x, int y, int rw, int rh, int r, uint16_t color) {
		assert(rw > 0 && rh > 0 && r >= 0 && r <= std::min(rw, rh) / 2);
		for (int row = y; row < y + rh; ++row) {
			for (int col = x; col < x + rw; ++col) {
				int cx = std::max(x + r, std::min(col, x + rw - r - 1));
				int cy = std::max(y + r, std::min(row, y + rh - r - 1));
				if (!r || (col - cx) * (col - cx) + (row - cy) * (row - cy) <= r * r) {
					pixel(col, row, color);
				}
			}
		}
	}
	void line(int x, int y, int end_x, int end_y, uint16_t color) {
		int dx = std::abs(end_x - x), dy = -std::abs(end_y - y);
		int sx = x < end_x ? 1 : -1, sy = y < end_y ? 1 : -1, error = dx + dy;
		for (;;) {
			pixel(x, y, color);
			if (x == end_x && y == end_y) { break; }
			int twice = 2 * error;
			if (twice >= dy) { error += dy; x += sx; }
			if (twice <= dx) { error += dx; y += sy; }
		}
	}
	static int64_t edge(int ax, int ay, int bx, int by, int px, int py) {
		return int64_t(px - ax) * (by - ay) - int64_t(py - ay) * (bx - ax);
	}
	void triangle(int ax, int ay, int bx, int by, int cx, int cy, uint16_t color) {
		if (!edge(ax, ay, bx, by, cx, cy)) { line(ax, ay, bx, by, color); line(bx, by, cx, cy, color); return; }
		for (int y = std::min({ay, by, cy}); y <= std::max({ay, by, cy}); ++y) {
			for (int x = std::min({ax, bx, cx}); x <= std::max({ax, bx, cx}); ++x) {
				int64_t a = edge(ax, ay, bx, by, x, y), b = edge(bx, by, cx, cy, x, y);
				int64_t c = edge(cx, cy, ax, ay, x, y);
				if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) { pixel(x, y, color); }
			}
		}
	}
};

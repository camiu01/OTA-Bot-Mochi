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

struct Raster {
	std::vector<uint16_t> pixels = std::vector<uint16_t>(240 * 280, 0);
	void pixel(int x, int y, uint16_t color) {
		if (x >= 0 && x < 240 && y >= 0 && y < 280) { pixels[y * 240 + x] = color; }
	}
	void rectangle(int x, int y, int w, int h, uint16_t color) {
		assert(w >= 0 && h >= 0);
		for (int row = std::max(y, 0); row < std::min(y + h, 280); ++row) {
			for (int col = std::max(x, 0); col < std::min(x + w, 240); ++col) { pixel(col, row, color); }
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
	void rounded(int x, int y, int w, int h, int r, uint16_t color) {
		assert(w > 0 && h > 0 && r >= 0 && r <= std::min(w, h) / 2);
		for (int row = y; row < y + h; ++row) {
			for (int col = x; col < x + w; ++col) {
				int cx = std::max(x + r, std::min(col, x + w - r - 1));
				int cy = std::max(y + r, std::min(row, y + h - r - 1));
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

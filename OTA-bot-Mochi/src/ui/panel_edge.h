/** @file panel_edge.h @brief Cached rounded paths for RGB565 perimeter effects. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cmath>
#include <stddef.h>
#include <stdint.h>
#include "../../pin_config.h"

namespace panel_edge {
constexpr bool Supported = MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448;

/** @brief Sample straight edges and rounded corners in clockwise order from the lower left. */
template<int Inset, int Radius, int InnerReach>
class Path {
	static constexpr int Corner = Inset + Radius;
	static constexpr int Right = LCD_WIDTH - 1 - Corner, Bottom = LCD_HEIGHT - 1 - Corner;
	static constexpr int W = Right - Corner, H = Bottom - Corner;
	static constexpr int Arc = int(Radius * 1.5707963f + 0.5f);
	static constexpr int Band = Inset + InnerReach;
	struct Sample { uint8_t opacity, along; };
	Sample corners_[Corner][Corner];
	uint8_t straight_[Band];

public:
	static constexpr int Perimeter = 2 * (W + H) + 4 * Arc;
	static constexpr size_t MaxSamples = 2 * Band * (W + H + 2) + 4 * Corner * Corner;

	explicit Path(uint8_t (*opacity)(float)) {
		for (int i = 0; i < Band; ++i) { straight_[i] = opacity(float(i - Inset)); }
		for (int y = 0; y < Corner; ++y) {
			for (int x = 0; x < Corner; ++x) {
				const float u = float(Corner - x), v = float(Corner - y);
				corners_[y][x] = {opacity(Radius - std::sqrt(u * u + v * v)),
					uint8_t(std::atan2(u, v) * (Arc / 1.5707963f) + 0.5f)};
			}
		}
	}

	template<typename Paint>
	void visit(Paint paint) const {
		for (int edge = 0; edge < Band; ++edge) {
			const auto light = straight_[edge];
			if (!light) { continue; }
			for (int y = Corner; y <= Bottom; ++y) {
				paint(edge, y, light, Arc + Bottom - y);
				paint(LCD_WIDTH - 1 - edge, y, light, 3 * Arc + H + W + y - Corner);
			}
			for (int x = Corner; x <= Right; ++x) {
				paint(x, edge, light, 2 * Arc + H + x - Corner);
				paint(x, LCD_HEIGHT - 1 - edge, light, 4 * Arc + 2 * H + W + Right - x);
			}
		}
		for (int y = 0; y < Corner; ++y) {
			for (int x = 0; x < Corner; ++x) {
				const auto s = corners_[y][x];
				if (!s.opacity) { continue; }
				const int right = LCD_WIDTH - 1 - x, bottom = LCD_HEIGHT - 1 - y;
				paint(x, bottom, s.opacity, s.along);
				paint(x, y, s.opacity, 2 * Arc + H - s.along);
				paint(right, y, s.opacity, 2 * Arc + H + W + s.along);
				paint(right, bottom, s.opacity, 4 * Arc + 2 * H + W - s.along);
			}
		}
	}
};
}

/**
 * @file calculating.h
 * @brief calculating definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace calculating_face {
using namespace face_primitives;

/** @brief calculating lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h * (b.left ? 30 : 15) / 100);
			strip(gfx, b, b.h * (b.left ? 25 : 10) / 100, true, cfg.bgColor);
}

/** @brief A bold plus and equals pair makes the concentrated face read as mental arithmetic. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)cut;
	int16_t half = int16_t(s * 2 / 3), stroke = int16_t(s / 5 + 1);
	if (b.left) {
		gfx->fillRect(x - half, y - stroke / 2, 2 * half + 1, stroke, color);
		gfx->fillRect(x - stroke / 2, y - half, stroke, 2 * half + 1, color);
	} else {
		gfx->fillRect(x - half, y - s / 3, 2 * half + 1, stroke, color);
		gfx->fillRect(x - half, y + s / 3, 2 * half + 1, stroke, color);
	}
	return true;
}

/** @brief Immutable calculating face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "calculating";
	p.lids = lids;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid calculating face parameters");
} }

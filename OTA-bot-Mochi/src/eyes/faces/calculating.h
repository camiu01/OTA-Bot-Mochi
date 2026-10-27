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

/** @brief calculating glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	hourglass(gfx, x, y, s, color); return true;
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

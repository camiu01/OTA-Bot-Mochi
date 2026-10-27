/**
 * @file low_batt.h
 * @brief low_batt definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace low_batt_face {
using namespace face_primitives;

/** @brief low_batt lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 4);
			strip(gfx, b, b.h / 5, false, cfg.bgColor);
}

/** @brief low_batt glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	low_batt(gfx, x, y, s, color); return true;
}

/** @brief Immutable low_batt face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "low_batt";
	p.lids = lids;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid low_batt face parameters");
} }

/**
 * @file target.h
 * @brief target definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace target_face {
using namespace face_primitives;

/** @brief Concentric red target rings and a central bullseye. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cut;
	s = int16_t(s > (b.w < b.h ? b.w : b.h) / 3 ? s : (b.w < b.h ? b.w : b.h) / 3);
	gfx->fillCircle(x, y, s, color);
	if (s > 3) { gfx->fillCircle(x, y, s - 2, eye_rgb(255, 88, 96)); }
	gfx->fillCircle(x, y, s / 2, cfg.fgColor);
	gfx->fillCircle(x, y, int16_t(s / 4 + 1), eye_rgb(255, 88, 96));
	return true;
}

/** @brief Immutable target face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "target";
	p.normalCycle = true;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid target face parameters");
} }

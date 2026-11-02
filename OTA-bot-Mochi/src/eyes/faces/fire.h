/**
 * @file fire.h
 * @brief fire definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace fire_face {
using namespace face_primitives;

/** @brief Orange flame eyes with a yellow inner tongue and configured edge. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	flame(gfx, x, y, s, color);
	if (s > 5) { flame(gfx, x, y, int16_t(s - 2), eye_rgb(255, 112, 32)); }
	if (s > 7) { flame(gfx, x, int16_t(y + s / 3), int16_t(s / 2), eye_rgb(255, 224, 64)); }
	return true;
}

/** @brief Immutable fire face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "fire";
	p.normalCycle = true;
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.wholeEye = p.replacesSolid = true;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid fire face parameters");
} }

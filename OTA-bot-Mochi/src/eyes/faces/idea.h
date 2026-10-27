/**
 * @file idea.h
 * @brief idea definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace idea_face {
using namespace face_primitives;

/** @brief idea glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	exclaim(gfx, x, y, s, color); return true;
}

/** @brief Immutable idea face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "idea";
	p.left.widthDelta = 4;
	p.right.widthDelta = 4;
	p.left.heightDelta = 4;
	p.right.heightDelta = 4;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid idea face parameters");
} }

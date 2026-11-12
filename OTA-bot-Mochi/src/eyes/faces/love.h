/**
 * @file love.h
 * @brief love definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace love_face {
using namespace face_primitives;

/** @brief Large love silhouette with configured outline and a bright colored center. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)cut;
	heart(gfx, x, y, s, color);
	if (s > 5) { heart(gfx, x, y, int16_t(s - 3), eye_rgb(255, 92, 150)); }
	return true;
}

/** @brief love reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)dx;
	uint32_t beat = phase % 1500;
	float pulse = beat < 240 ? sinf(beat * 0.0130899694f) :
		(beat >= 320 && beat < 560 ? 0.6f * sinf((beat - 320) * 0.0130899694f) : 0);
	sx += a * 0.13f * pulse; sy += a * 0.18f * pulse; dy -= a * 2 * pulse;
}

/** @brief Immutable love face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "love";
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.wholeEye = true;
	p.replacesSolid = true;
	p.hasReactionInk = true;
	p.reactionInk = eye_rgb(255, 92, 150);
	p.glyph = glyph;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid love face parameters");
} }

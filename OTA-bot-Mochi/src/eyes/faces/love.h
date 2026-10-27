/**
 * @file love.h
 * @brief love definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace love_face {
using namespace face_primitives;

/** @brief love glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	heart(gfx, x, y, s, color); return true;
}

/** @brief love reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = LOVE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * 0.12f * wave; sy += a * 0.12f * wave;
}

/** @brief Immutable love face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "love";
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

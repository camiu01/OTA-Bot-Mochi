/**
 * @file nervous.h
 * @brief nervous definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace nervous_face {
using namespace face_primitives;

/** @brief nervous lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 6);
			strip(gfx, b, b.h / 8, false, cfg.bgColor);
}

/** @brief nervous reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = NERVOUS; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dx = a * 3 * sinf(phase * 0.031415927f + (left ? 0 : 1.5707963f));
			sy += a * 0.08f;
}

/** @brief Immutable nervous face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "nervous";
	p.left.widthScale = 0.82f;
	p.right.widthScale = 0.82f;
	p.left.heightScale = 0.88f;
	p.right.heightScale = 0.88f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid nervous face parameters");
} }

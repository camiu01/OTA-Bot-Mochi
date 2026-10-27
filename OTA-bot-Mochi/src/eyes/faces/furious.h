/**
 * @file furious.h
 * @brief furious definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace furious_face {
using namespace face_primitives;

/** @brief furious lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h / 2);
}

/** @brief furious reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = FURIOUS; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * (expr == FURIOUS ? 0.3f : 0.15f);
			dx = a * (expr == FURIOUS ? 2 : 1) * sinf(phase * 0.025132741f);
}

/** @brief Immutable furious face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "furious";
	p.left.widthDelta = 10;
	p.right.widthDelta = 10;
	p.left.heightScale = 0.48f;
	p.right.heightScale = 0.48f;
	p.left.offsetY = 3;
	p.right.offsetY = 3;
	p.hasReactionInk = true;
	p.reactionInk = eye_rgb(245, 70, 85);
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid furious face parameters");
} }

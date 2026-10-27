/**
 * @file happy.h
 * @brief happy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace happy_face {
using namespace face_primitives;

/** @brief happy lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	smile(gfx, cfg, b, b.h / 6, b.h / 3);
}

/** @brief happy reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = HAPPY; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = -a * (3 + 2 * wave); sx += a * 0.05f;
}

/** @brief Immutable happy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "happy";
	p.left.widthDelta = 4;
	p.right.widthDelta = 4;
	p.left.heightScale = 0.65f;
	p.right.heightScale = 0.65f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid happy face parameters");
} }

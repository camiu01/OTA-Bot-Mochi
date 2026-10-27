/**
 * @file mild_happy.h
 * @brief mild_happy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace mild_happy_face {
using namespace face_primitives;

/** @brief mild_happy lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	smile(gfx, cfg, b, b.h / 10, b.h / 8);
}

/** @brief mild_happy reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = MILD_HAPPY; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = -a * (3 + 2 * wave); sx += a * 0.05f;
}

/** @brief Immutable mild_happy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "mild_happy";
	p.left.heightScale = 0.88f;
	p.right.heightScale = 0.88f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid mild_happy face parameters");
} }

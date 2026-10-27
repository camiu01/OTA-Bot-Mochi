/**
 * @file smirk.h
 * @brief smirk definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace smirk_face {
using namespace face_primitives;

/** @brief smirk lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 45 : 25) / 100, false, cfg.bgColor);
}

/** @brief smirk reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SMIRK; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * (left ? 0.1f : 0.5f); dy = a * (left ? -4 : 3);
}

/** @brief Immutable smirk face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "smirk";
	p.left.asym.heightScale = 0.70f;
	p.right.asym.heightScale = 1.00f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid smirk face parameters");
} }

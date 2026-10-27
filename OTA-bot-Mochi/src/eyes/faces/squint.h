/**
 * @file squint.h
 * @brief squint definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace squint_face {
using namespace face_primitives;

/** @brief squint lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, false, cfg.bgColor);
}

/** @brief squint reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SQUINT; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * 0.7f; sx += a * 0.08f;
}

/** @brief Immutable squint face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "squint";
	p.left.heightScale = 0.45f;
	p.right.heightScale = 0.45f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid squint face parameters");
} }

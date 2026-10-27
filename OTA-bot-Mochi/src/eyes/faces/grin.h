/**
 * @file grin.h
 * @brief grin definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace grin_face {
using namespace face_primitives;

/** @brief grin lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	smile(gfx, cfg, b, b.h / 8, b.h / 4);
}

/** @brief grin reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = GRIN; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = -a * (3 + 2 * wave); sx += a * 0.05f;
}

/** @brief Immutable grin face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "grin";
	p.left.widthDelta = 12;
	p.right.widthDelta = 12;
	p.left.heightScale = 0.78f;
	p.right.heightScale = 0.78f;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid grin face parameters");
} }

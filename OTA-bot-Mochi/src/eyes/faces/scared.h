/**
 * @file scared.h
 * @brief scared definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace scared_face {
using namespace face_primitives;

/** @brief scared reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SCARED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dx = a * 3 * sinf(phase * 0.031415927f + (left ? 0 : 1.5707963f));
			sy += a * 0.08f;
}

/** @brief Immutable scared face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "scared";
	p.left.widthScale = 0.60f;
	p.right.widthScale = 0.60f;
	p.left.heightDelta = 12;
	p.right.heightDelta = 12;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid scared face parameters");
} }

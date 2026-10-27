/**
 * @file awe.h
 * @brief awe definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace awe_face {
using namespace face_primitives;

/** @brief awe reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = AWE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * 0.12f; sy += a * 0.15f; dy = -a * 5;
}

/** @brief Immutable awe face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "awe";
	p.left.widthDelta = 12;
	p.right.widthDelta = 12;
	p.left.heightFromWidth = true;
	p.right.heightFromWidth = true;
	p.left.offsetY = -6;
	p.right.offsetY = -6;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid awe face parameters");
} }

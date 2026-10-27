/**
 * @file surprised.h
 * @brief surprised definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace surprised_face {
using namespace face_primitives;

/** @brief surprised reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SURPRISED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * 0.12f; sy += a * 0.15f; dy = -a * 5;
}

/** @brief Immutable surprised face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "surprised";
	p.left.widthDelta = 8;
	p.right.widthDelta = 8;
	p.left.heightDelta = 12;
	p.right.heightDelta = 12;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid surprised face parameters");
} }

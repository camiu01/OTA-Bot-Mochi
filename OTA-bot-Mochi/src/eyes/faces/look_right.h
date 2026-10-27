/**
 * @file look_right.h
 * @brief look_right definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace look_right_face {
using namespace face_primitives;

/** @brief look_right performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = LOOK_RIGHT; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dx = a * 18;
}

/** @brief Immutable look_right face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "look_right";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(245, 165, 205);
	p.performanceMotion = performanceMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid look_right face parameters");
} }

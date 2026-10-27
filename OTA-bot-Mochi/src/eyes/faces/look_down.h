/**
 * @file look_down.h
 * @brief look_down definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace look_down_face {
using namespace face_primitives;

/** @brief look_down performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = LOOK_DOWN; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = a * 16; sy -= a * 0.15f;
}

/** @brief Immutable look_down face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "look_down";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(175, 150, 245);
	p.performanceMotion = performanceMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid look_down face parameters");
} }

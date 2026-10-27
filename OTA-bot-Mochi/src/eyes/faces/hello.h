/**
 * @file hello.h
 * @brief hello definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace hello_face {
using namespace face_primitives;

/** @brief hello performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = HELLO; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = -a * 7 * fabsf(wave); if (!left) { sy -= a * 0.8f * (0.5f + 0.5f * wave); }
}

/** @brief Immutable hello face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "hello";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(255, 175, 120);
	p.performanceMotion = performanceMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid hello face parameters");
} }

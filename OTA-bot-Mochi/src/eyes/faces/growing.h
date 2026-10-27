/**
 * @file growing.h
 * @brief growing definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace growing_face {
using namespace face_primitives;

/** @brief growing performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = GROWING; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * (0.22f + 0.08f * wave); sy += a * 0.22f;
}

/** @brief Immutable growing face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "growing";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(120, 235, 150);
	p.performanceMotion = performanceMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid growing face parameters");
} }

/**
 * @file distracted.h
 * @brief distracted definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace distracted_face {
using namespace face_primitives;

/** @brief distracted performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = DISTRACTED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dx = a * 14 * sinf(phase * 0.0020943951f + (left ? 0 : 1.3f)); dy = a * (left ? -5 : 5);
}

/** @brief Immutable distracted face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "distracted";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(130, 225, 210);
	p.performanceMotion = performanceMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid distracted face parameters");
} }

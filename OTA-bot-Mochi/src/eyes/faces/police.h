/**
 * @file police.h
 * @brief police definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace police_face {
using namespace face_primitives;

/** @brief police performanceColor procedure. */
inline uint16_t performanceColor(const EyeConfig &cfg, bool left, uint32_t elapsed) {
	(void)cfg;
	float wave = 0.5f + 0.5f * sinf((elapsed % 6000) * 0.0020943951f);
	if (!left) { wave = 1 - wave; }
	return eye_mix(eye_rgb(245, 70, 90), eye_rgb(85, 145, 245), uint8_t(wave * 100));
}

/** @brief Immutable police face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "police";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(245, 70, 90);
	p.performanceColor = performanceColor;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid police face parameters");
} }

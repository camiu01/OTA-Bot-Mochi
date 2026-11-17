/**
 * @file kiss.h
 * @brief kiss definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace kiss_face {
using namespace face_primitives;

/** @brief kiss lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	smile(gfx, cfg, b, b.h / 3, b.h / 3);
	if (b.left || b.h < 16) { return; }
	int16_t s = eye_limit(b.w / 5, 2, 10);
	heart(gfx, b.x + b.w - s - 1, b.y + b.h - s - 2, s, eye_rgb(255, 110, 150));
}

/** @brief Immutable kiss face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "kiss";
	p.left.widthScale = 0.85f;
	p.right.widthScale = 0.85f;
	p.left.heightScale = 0.70f;
	p.right.heightScale = 0.70f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid kiss face parameters");
} }

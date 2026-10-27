/**
 * @file cheeky.h
 * @brief cheeky definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace cheeky_face {
using namespace face_primitives;

/** @brief cheeky lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 3, true, cfg.bgColor);
			slope(gfx, cfg, b, true, b.h / 6);
}

/** @brief Immutable cheeky face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "cheeky";
	p.left.asym.heightScale = 1.00f;
	p.right.asym.heightScale = 0.18f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid cheeky face parameters");
} }

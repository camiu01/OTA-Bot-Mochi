/**
 * @file melting.h
 * @brief melting definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace melting_face {
using namespace face_primitives;

/** @brief melting lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 4);
}

/** @brief Immutable melting face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "melting";
	p.left.asym.heightScale = 1.00f;
	p.left.asym.offsetY = 0;
	p.right.asym.heightScale = 0.50f;
	p.right.asym.offsetY = 6;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid melting face parameters");
} }

/**
 * @file zombie.h
 * @brief zombie definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace zombie_face {
using namespace face_primitives;

/** @brief zombie lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 40 : 10) / 100, false, cfg.bgColor);
}

/** @brief Immutable zombie face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "zombie";
	p.left.asym.heightScale = 0.55f;
	p.right.asym.heightScale = 1.00f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid zombie face parameters");
} }

/**
 * @file tired.h
 * @brief tired definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace tired_face {
using namespace face_primitives;

/** @brief Outer corners droop under heavy lids above a flat lower cut. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 2);
	strip(gfx, b, b.h / 5, false, cfg.bgColor);
	strip(gfx, b, b.h / 4, true, cfg.bgColor);
}

/** @brief Immutable tired face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "tired";
	p.left.heightScale = 0.70f;
	p.right.heightScale = 0.70f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid tired face parameters");
} }

/**
 * @file proud.h
 * @brief proud definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace proud_face {
using namespace face_primitives;

/** @brief proud lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 30 : 40) / 100, false, cfg.bgColor);
}

/** @brief Immutable proud face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "proud";
	p.left.asym.heightScale = 0.85f;
	p.right.asym.heightScale = 0.70f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid proud face parameters");
} }

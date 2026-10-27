/**
 * @file pout.h
 * @brief pout definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace pout_face {
using namespace face_primitives;

/** @brief pout lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 8, false, cfg.bgColor);
			strip(gfx, b, b.h / 8, true, cfg.bgColor);
}

/** @brief Immutable pout face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "pout";
	p.left.widthDelta = 4;
	p.right.widthDelta = 4;
	p.left.heightScale = 0.85f;
	p.right.heightScale = 0.85f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid pout face parameters");
} }

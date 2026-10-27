/**
 * @file groggy.h
 * @brief groggy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace groggy_face {
using namespace face_primitives;

/** @brief groggy lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 15 : 45) / 100, false, cfg.bgColor);
}

/** @brief Immutable groggy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "groggy";
	p.left.asym.heightScale = 1.00f;
	p.left.asym.offsetY = 0;
	p.right.asym.heightScale = 0.55f;
	p.right.asym.offsetY = 4;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid groggy face parameters");
} }

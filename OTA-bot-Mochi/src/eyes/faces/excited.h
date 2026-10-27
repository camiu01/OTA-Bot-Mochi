/**
 * @file excited.h
 * @brief excited definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace excited_face {
using namespace face_primitives;

/** @brief excited lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 6, true, cfg.bgColor);
}

/** @brief Immutable excited face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "excited";
	p.left.widthDelta = 8;
	p.right.widthDelta = 8;
	p.left.heightDelta = 10;
	p.right.heightDelta = 10;
	p.left.offsetY = -4;
	p.right.offsetY = -4;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid excited face parameters");
} }

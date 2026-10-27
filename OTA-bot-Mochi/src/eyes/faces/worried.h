/**
 * @file worried.h
 * @brief worried definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace worried_face {
using namespace face_primitives;

/** @brief worried lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 5);
}

/** @brief Immutable worried face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "worried";
	p.left.widthScale = 0.85f;
	p.right.widthScale = 0.85f;
	p.left.heightScale = 0.95f;
	p.right.heightScale = 0.95f;
	p.left.offsetY = -5;
	p.right.offsetY = -5;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid worried face parameters");
} }

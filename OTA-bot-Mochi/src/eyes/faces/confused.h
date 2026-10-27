/**
 * @file confused.h
 * @brief confused definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace confused_face {
using namespace face_primitives;

/** @brief confused lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h * (b.left ? 12 : 34) / 100);
}

/** @brief Immutable confused face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "confused";
	p.left.asym.widthDelta = 6;
	p.left.asym.heightDelta = 8;
	p.right.asym.widthDelta = -6;
	p.right.asym.heightDelta = -10;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid confused face parameters");
} }

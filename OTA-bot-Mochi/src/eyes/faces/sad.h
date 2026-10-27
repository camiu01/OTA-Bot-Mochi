/**
 * @file sad.h
 * @brief sad definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sad_face {
using namespace face_primitives;

/** @brief sad lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 3);
}

/** @brief Immutable sad face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sad";
	p.left.widthDelta = -4;
	p.right.widthDelta = -4;
	p.left.heightScale = 0.82f;
	p.right.heightScale = 0.82f;
	p.left.offsetY = 5;
	p.right.offsetY = 5;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sad face parameters");
} }

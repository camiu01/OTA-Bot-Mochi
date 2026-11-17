/**
 * @file glum.h
 * @brief glum definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace glum_face {
using namespace face_primitives;

/** @brief A steep outward sag, lowered in the panel, with no smile. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h * 3 / 5);
}

/** @brief Immutable glum face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "glum";
	p.left.heightScale = 0.75f;
	p.right.heightScale = 0.75f;
	p.left.offsetY = 8;
	p.right.offsetY = 8;
	p.gazeY = 0.3f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid glum face parameters");
} }

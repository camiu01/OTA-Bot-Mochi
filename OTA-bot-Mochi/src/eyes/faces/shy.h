/**
 * @file shy.h
 * @brief shy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace shy_face {
using namespace face_primitives;

/** @brief shy lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, true, cfg.bgColor);
}

/** @brief Immutable shy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "shy";
	p.left.widthScale = 0.75f;
	p.right.widthScale = 0.75f;
	p.left.heightScale = 0.62f;
	p.right.heightScale = 0.62f;
	p.left.offsetY = 10;
	p.right.offsetY = 10;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid shy face parameters");
} }

/**
 * @file squish.h
 * @brief squish definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace squish_face {
using namespace face_primitives;

/** @brief squish lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, true, cfg.bgColor);
}

/** @brief Immutable squish face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "squish";
	p.left.widthDelta = 10;
	p.right.widthDelta = 10;
	p.left.heightScale = 0.65f;
	p.right.heightScale = 0.65f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid squish face parameters");
} }

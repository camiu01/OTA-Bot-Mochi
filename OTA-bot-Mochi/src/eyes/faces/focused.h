/**
 * @file focused.h
 * @brief focused definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace focused_face {
using namespace face_primitives;

/** @brief focused lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 5, false, cfg.bgColor);
	strip(gfx, b, b.h / 5, true, cfg.bgColor);
}

/** @brief Immutable focused face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "focused";
	p.left.heightScale = 0.48f;
	p.right.heightScale = 0.48f;
	p.left.widthDelta = 8;
	p.right.widthDelta = 8;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid focused face parameters");
} }

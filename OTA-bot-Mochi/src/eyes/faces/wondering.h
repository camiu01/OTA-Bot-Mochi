/**
 * @file wondering.h
 * @brief wondering definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace wondering_face {
using namespace face_primitives;

/** @brief wondering lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 6, false, cfg.bgColor);
			strip(gfx, b, b.h / 10, true, cfg.bgColor);
}

/** @brief Immutable wondering face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "wondering";
	p.left.asym.offsetY = 0;
	p.right.asym.offsetY = -5;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid wondering face parameters");
} }

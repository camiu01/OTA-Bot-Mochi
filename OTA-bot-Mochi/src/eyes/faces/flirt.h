/**
 * @file flirt.h
 * @brief flirt definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace flirt_face {
using namespace face_primitives;

/** @brief flirt lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, false, cfg.bgColor);
			strip(gfx, b, b.h / 6, true, cfg.bgColor);
}

/** @brief Immutable flirt face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "flirt";
	p.left.asym.heightScale = 0.88f;
	p.left.asym.offsetY = 0;
	p.right.asym.heightScale = 0.66f;
	p.right.asym.offsetY = 3;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid flirt face parameters");
} }

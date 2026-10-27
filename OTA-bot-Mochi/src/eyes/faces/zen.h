/**
 * @file zen.h
 * @brief zen definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace zen_face {
using namespace face_primitives;

/** @brief zen lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * 40 / 100, false, cfg.bgColor);
			strip(gfx, b, b.h * 40 / 100, true, cfg.bgColor);
}

/** @brief Immutable zen face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "zen";
	p.left.widthDelta = 8;
	p.right.widthDelta = 8;
	p.left.heightScale = 0.38f;
	p.right.heightScale = 0.38f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid zen face parameters");
} }

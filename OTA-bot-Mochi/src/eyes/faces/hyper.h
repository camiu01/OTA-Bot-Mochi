/**
 * @file hyper.h
 * @brief hyper definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace hyper_face {
using namespace face_primitives;

/** @brief hyper lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 12, true, cfg.bgColor);
}

/** @brief Immutable hyper face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "hyper";
	p.left.widthDelta = 6;
	p.right.widthDelta = 6;
	p.left.heightDelta = 6;
	p.right.heightDelta = 6;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid hyper face parameters");
} }

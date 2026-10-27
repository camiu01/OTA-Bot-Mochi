/**
 * @file panicked.h
 * @brief panicked definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace panicked_face {
using namespace face_primitives;

/** @brief panicked lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 8, false, cfg.bgColor);
			strip(gfx, b, b.h / 8, true, cfg.bgColor);
}

/** @brief Immutable panicked face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "panicked";
	p.left.widthDelta = 6;
	p.right.widthDelta = 6;
	p.left.heightDelta = 14;
	p.right.heightDelta = 14;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid panicked face parameters");
} }

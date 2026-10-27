/**
 * @file daydream.h
 * @brief daydream definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace daydream_face {
using namespace face_primitives;

/** @brief daydream lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 6, false, cfg.bgColor);
}

/** @brief Immutable daydream face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "daydream";
	p.left.widthDelta = 6;
	p.right.widthDelta = 6;
	p.left.heightScale = 0.92f;
	p.right.heightScale = 0.92f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid daydream face parameters");
} }

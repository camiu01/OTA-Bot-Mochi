/**
 * @file sad_up.h
 * @brief sad_up definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sad_up_face {
using namespace face_primitives;

/** @brief sad_up lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, true, cfg.bgColor);
		// WINK keeps a visible closed eye even when per-eye geometry is disabled.
}

/** @brief Immutable sad_up face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sad_up";
	p.left.widthDelta = 2;
	p.right.widthDelta = 2;
	p.left.heightScale = 0.90f;
	p.right.heightScale = 0.90f;
	p.left.offsetY = -8;
	p.right.offsetY = -8;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sad_up face parameters");
} }

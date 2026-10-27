/**
 * @file sad_down.h
 * @brief sad_down definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sad_down_face {
using namespace face_primitives;

/** @brief sad_down lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 3);
			strip(gfx, b, b.h / 6, false, cfg.bgColor);
}

/** @brief Immutable sad_down face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sad_down";
	p.left.widthDelta = -6;
	p.right.widthDelta = -6;
	p.left.heightScale = 0.65f;
	p.right.heightScale = 0.65f;
	p.left.offsetY = 12;
	p.right.offsetY = 12;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sad_down face parameters");
} }

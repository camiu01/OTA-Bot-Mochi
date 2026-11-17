/**
 * @file cat.h
 * @brief cat definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace cat_face {
using namespace face_primitives;

/** @brief Almond feline lids: inner top and outer bottom cuts tilt each eye upward. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h / 3);
	int16_t outer = b.left ? b.x : b.x + b.w - 1, inner = b.left ? b.x + b.w - 1 : b.x, base = b.y + b.h - 1;
	gfx->fillTriangle(inner, base, outer, base, outer, base - b.h / 3, cfg.bgColor);
}

/** @brief Immutable cat face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "cat";
	p.left.widthDelta = 6;
	p.right.widthDelta = 6;
	p.left.heightScale = 0.8f;
	p.right.heightScale = 0.8f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid cat face parameters");
} }

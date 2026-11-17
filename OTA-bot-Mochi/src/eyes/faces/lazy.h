/**
 * @file lazy.h
 * @brief lazy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace lazy_face {
using namespace face_primitives;

/** @brief Uneven heavy lids: the left eye is nearly shut, the right half-open. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 55 : 30) / 100, false, cfg.bgColor);
	strip(gfx, b, b.h / 6, true, cfg.bgColor);
}

/** @brief Immutable lazy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "lazy";
	p.left.widthDelta = 6;
	p.right.widthDelta = 6;
	p.left.heightScale = 0.55f;
	p.right.heightScale = 0.55f;
	p.left.offsetY = 8;
	p.right.offsetY = 3;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid lazy face parameters");
} }

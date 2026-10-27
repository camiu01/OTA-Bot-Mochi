/**
 * @file annoyed.h
 * @brief annoyed definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace annoyed_face {
using namespace face_primitives;

/** @brief annoyed lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 25 : 35) / 100, false, cfg.bgColor);
}

/** @brief Immutable annoyed face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "annoyed";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid annoyed face parameters");
} }

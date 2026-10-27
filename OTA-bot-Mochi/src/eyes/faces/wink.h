/**
 * @file wink.h
 * @brief wink definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace wink_face {
using namespace face_primitives;

/** @brief wink lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 0 : 85) / 100, false, cfg.bgColor);
}

/** @brief Immutable wink face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "wink";
	p.right.asym.height = 6;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid wink face parameters");
} }

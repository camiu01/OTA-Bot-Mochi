/**
 * @file derp.h
 * @brief derp definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace derp_face {
using namespace face_primitives;

/** @brief derp lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 10 : 25) / 100, b.left, cfg.bgColor);
}

/** @brief Immutable derp face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "derp";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid derp face parameters");
} }

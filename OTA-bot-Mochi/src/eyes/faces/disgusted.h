/**
 * @file disgusted.h
 * @brief disgusted definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace disgusted_face {
using namespace face_primitives;

/** @brief disgusted lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h / 5);
			strip(gfx, b, b.h / 6, true, cfg.bgColor);
}

/** @brief Immutable disgusted face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "disgusted";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid disgusted face parameters");
} }

/**
 * @file side_eye.h
 * @brief side_eye definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace side_eye_face {
using namespace face_primitives;

/** @brief side_eye lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 6 : 26) / 100, false, cfg.bgColor);
}

/** @brief Immutable side_eye face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "side_eye";
	p.sideGaze = true;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid side_eye face parameters");
} }

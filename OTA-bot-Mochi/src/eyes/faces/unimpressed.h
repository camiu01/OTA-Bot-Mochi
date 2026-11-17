/**
 * @file unimpressed.h
 * @brief unimpressed definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace unimpressed_face {
using namespace face_primitives;

/** @brief Half lids with a flat lower cut while the gaze rolls upward. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 2, false, cfg.bgColor);
	strip(gfx, b, b.h / 8, true, cfg.bgColor);
}

/** @brief Immutable unimpressed face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "unimpressed";
	p.gazeY = -0.45f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid unimpressed face parameters");
} }

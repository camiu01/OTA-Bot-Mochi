/**
 * @file frustrated.h
 * @brief frustrated definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace frustrated_face {
using namespace face_primitives;

/** @brief frustrated lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h * (b.left ? 20 : 30) / 100);
}

/** @brief Immutable frustrated face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "frustrated";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid frustrated face parameters");
} }

/**
 * @file suspicious.h
 * @brief suspicious definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace suspicious_face {
using namespace face_primitives;

/** @brief suspicious lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 15 : 35) / 100, false, cfg.bgColor);
}

/** @brief Immutable suspicious face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "suspicious";
	p.left.asym.offsetY = -5;
	p.right.asym.offsetY = 5;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid suspicious face parameters");
} }

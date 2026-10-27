/**
 * @file skeptic.h
 * @brief skeptic definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace skeptic_face {
using namespace face_primitives;

/** @brief skeptic lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 10 : 35) / 100, false, cfg.bgColor);
}

/** @brief Immutable skeptic face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "skeptic";
	p.left.asym.widthDelta = 4;
	p.left.asym.heightDelta = 6;
	p.right.asym.widthDelta = -8;
	p.right.asym.heightDelta = -16;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid skeptic face parameters");
} }

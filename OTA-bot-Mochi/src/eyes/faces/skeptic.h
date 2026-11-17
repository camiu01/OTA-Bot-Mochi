/**
 * @file skeptic.h
 * @brief skeptic definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace skeptic_face {
using namespace face_primitives;

/** @brief One open eye beside a smaller eye with an inward-lowered lid. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (b.left) { strip(gfx, b, b.h / 10, false, cfg.bgColor); }
	else { slope(gfx, cfg, b, true, b.h / 2); strip(gfx, b, b.h / 5, false, cfg.bgColor); }
}

/** @brief Immutable skeptic face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "skeptic";
	p.left.heightScale = 1.08f;
	p.right.heightScale = 0.72f;
	p.right.widthDelta = -6;
	p.left.asym.widthDelta = 4;
	p.left.asym.heightDelta = 6;
	p.right.asym.widthDelta = -8;
	p.right.asym.heightDelta = -16;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid skeptic face parameters");
} }

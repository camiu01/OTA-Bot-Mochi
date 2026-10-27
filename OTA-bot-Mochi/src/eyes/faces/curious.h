/**
 * @file curious.h
 * @brief curious definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace curious_face {
using namespace face_primitives;

/** @brief curious lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 6, false, cfg.bgColor);
}

/** @brief Immutable curious face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "curious";
	p.left.asym.offsetY = 0;
	p.right.asym.offsetY = -6;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid curious face parameters");
} }

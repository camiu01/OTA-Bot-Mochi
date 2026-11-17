/**
 * @file merciful.h
 * @brief merciful definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace merciful_face {
using namespace face_primitives;

/** @brief Gentle outward sag above smiling lower edges: kind, not sad. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 4);
	smile(gfx, cfg, b, b.h / 12, b.h / 4);
}

/** @brief Immutable merciful face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "merciful";
	p.left.heightScale = 0.85f;
	p.right.heightScale = 0.85f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid merciful face parameters");
} }

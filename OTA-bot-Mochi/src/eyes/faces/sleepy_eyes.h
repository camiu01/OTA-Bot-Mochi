/**
 * @file sleepy_eyes.h
 * @brief sleepy_eyes definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sleepy_eyes_face {
using namespace face_primitives;

/** @brief sleepy_eyes lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 3, false, cfg.bgColor);
}

/** @brief Immutable sleepy_eyes face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sleepy_eyes";
	p.left.asym.heightScale = 0.45f;
	p.right.asym.heightScale = 0.30f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sleepy_eyes face parameters");
} }

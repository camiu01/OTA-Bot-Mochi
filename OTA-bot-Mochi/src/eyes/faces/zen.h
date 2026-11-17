/**
 * @file zen.h
 * @brief zen definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_ambient_backgrounds.h"

namespace face_defs { namespace zen_face {
using namespace face_primitives;

/** @brief Closed, peaceful downward arcs: a deep curved lid leaves a calm lower bowl. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	arch(gfx, cfg, b, b.h * 20 / 100, b.h * 50 / 100);
}

/** @brief Immutable zen face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "zen";
	p.background = face_backgrounds::ripples;
	p.bounds = face_backgrounds::full_bounds;
	p.left.widthDelta = 8;
	p.right.widthDelta = 8;
	p.left.heightScale = 0.5f;
	p.right.heightScale = 0.5f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid zen face parameters");
} }

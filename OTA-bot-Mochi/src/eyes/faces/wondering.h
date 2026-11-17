/**
 * @file wondering.h
 * @brief wondering definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace wondering_face {
using namespace face_primitives;

/** @brief Raised, curved upper lids as the gaze drifts up and aside. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	arch(gfx, cfg, b, b.h / 10, -(b.h / 6));
}

/** @brief Immutable wondering face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "wondering";
	p.gazeX = 0.35f;
	p.gazeY = -0.6f;
	p.left.asym.offsetY = 0;
	p.right.asym.offsetY = -5;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid wondering face parameters");
} }

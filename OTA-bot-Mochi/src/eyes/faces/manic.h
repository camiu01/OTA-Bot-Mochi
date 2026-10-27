/**
 * @file manic.h
 * @brief manic definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace manic_face {
using namespace face_primitives;

/** @brief manic lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 10, false, cfg.bgColor);
}

/** @brief Immutable manic face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "manic";
	p.left.widthDelta = 10;
	p.right.widthDelta = 10;
	p.left.heightDelta = 18;
	p.right.heightDelta = 2;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid manic face parameters");
} }

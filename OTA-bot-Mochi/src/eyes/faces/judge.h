/**
 * @file judge.h
 * @brief judge definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace judge_face {
using namespace face_primitives;

/** @brief judge lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * 35 / 100, false, cfg.bgColor);
			strip(gfx, b, b.h * 20 / 100, true, cfg.bgColor);
}

/** @brief Immutable judge face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "judge";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid judge face parameters");
} }

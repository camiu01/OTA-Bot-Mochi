/**
 * @file cyborg.h
 * @brief cyborg definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace cyborg_face {
using namespace face_primitives;

/** @brief cyborg lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (b.left) {
				slope(gfx, cfg, b, true, b.h / 4);
				strip(gfx, b, b.h / 5, true, cfg.bgColor);
			}
}

/** @brief Immutable cyborg face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "cyborg";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid cyborg face parameters");
} }

/**
 * @file sassy.h
 * @brief sassy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sassy_face {
using namespace face_primitives;

/** @brief A half-lidded, outward-tilted left eye beside a raised, wide-open right eye. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (b.left) { slope(gfx, cfg, b, false, b.h / 2); strip(gfx, b, b.h * 3 / 10, false, cfg.bgColor); }
	else { strip(gfx, b, b.h / 12, false, cfg.bgColor); }
}

/** @brief sassy reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)phase; (void)sx; (void)dx;
	sy -= a * (left ? 0.1f : 0.5f); dy = a * (left ? -4 : 3);
}

/** @brief Immutable sassy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sassy";
	p.right.heightScale = 1.12f;
	p.left.asym.offsetY = -5;
	p.right.asym.offsetY = 1;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sassy face parameters");
} }

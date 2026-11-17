/**
 * @file squint.h
 * @brief squint definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace squint_face {
using namespace face_primitives;

/** @brief Inner-lowered lids above cheek-raised lower edges: a pinched lens, not a flat bar. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h / 3);
	smile(gfx, cfg, b, b.h / 8, b.h / 4);
}

/** @brief squint reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)dx; (void)dy;
	sy -= a * 0.7f; sx += a * 0.08f;
}

/** @brief Immutable squint face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "squint";
	p.left.heightScale = 0.6f;
	p.right.heightScale = 0.6f;
	p.left.widthDelta = 4;
	p.right.widthDelta = 4;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid squint face parameters");
} }

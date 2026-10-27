/**
 * @file terrified.h
 * @brief terrified definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace terrified_face {
using namespace face_primitives;

/** @brief terrified lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 8, false, cfg.bgColor);
}

/** @brief terrified reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = TERRIFIED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dx = a * 3 * sinf(phase * 0.031415927f + (left ? 0 : 1.5707963f));
			sy += a * 0.08f;
}

/** @brief Immutable terrified face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "terrified";
	p.left.widthDelta = -6;
	p.right.widthDelta = -6;
	p.left.heightDelta = 24;
	p.right.heightDelta = 24;
	p.left.offsetY = -5;
	p.right.offsetY = -5;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid terrified face parameters");
} }

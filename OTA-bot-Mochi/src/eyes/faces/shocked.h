/**
 * @file shocked.h
 * @brief shocked definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace shocked_face {
using namespace face_primitives;

/** @brief shocked lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 12, false, cfg.bgColor);
}

/** @brief shocked reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SHOCKED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * 0.12f; sy += a * 0.15f; dy = -a * 5;
}

/** @brief Immutable shocked face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "shocked";
	p.left.widthDelta = 12;
	p.right.widthDelta = 12;
	p.left.heightDelta = 24;
	p.right.heightDelta = 24;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid shocked face parameters");
} }

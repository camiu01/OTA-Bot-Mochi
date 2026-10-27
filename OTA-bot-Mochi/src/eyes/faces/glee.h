/**
 * @file glee.h
 * @brief glee definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace glee_face {
using namespace face_primitives;

/** @brief glee lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	smile(gfx, cfg, b, b.h / 4, b.h / 2);
}

/** @brief glee reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = GLEE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = -a * 5 * fabsf(wave); sx += a * 0.12f; sy -= a * 0.15f;
}

/** @brief glee reactionBody procedure. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)st; (void)neutral;
	if (eye_reaction_amount(elapsed) < 0.8f) { return false; }
	eye_reaction_chevron(gfx, cfg, b); return true;
}

/** @brief Immutable glee face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "glee";
	p.left.widthDelta = 10;
	p.right.widthDelta = 10;
	p.left.heightScale = 0.48f;
	p.right.heightScale = 0.48f;
	p.left.offsetY = -4;
	p.right.offsetY = -4;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid glee face parameters");
} }

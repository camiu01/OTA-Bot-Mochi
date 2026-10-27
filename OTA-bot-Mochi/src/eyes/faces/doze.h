/**
 * @file doze.h
 * @brief doze definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace doze_face {
using namespace face_primitives;
inline void local_eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b);


/** @brief doze lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 2, false, cfg.bgColor);
			strip(gfx, b, b.h / 4, true, cfg.bgColor);
}

/** @brief doze reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = DOZE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	dy = a * (4 + 2 * wave);
}

/** @brief doze local_eye_reaction_rest implementation. */
inline void local_eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t thick = eye_limit(b.h / 8, 2, 6);
	for (int16_t x = 1; x < b.w - 1; ++x) {
		int32_t t = 2 * x - b.w + 1;
		int16_t curve = int16_t(t * t * b.h / (int32_t(b.w) * b.w * 3));
		int16_t y = cfg.expression == DOZE ? b.h * 2 / 3 - curve : b.h / 2 + curve / 2;
		gfx->fillRect(b.x + x, b.y + y, 1, thick, cfg.fgColor);
	}
}

/** @brief doze reactionBody procedure. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)st; (void)neutral;
	if (eye_reaction_amount(elapsed) < 0.8f) { return false; }
	local_eye_reaction_rest(gfx, cfg, b); return true;
}

/** @brief Immutable doze face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "doze";
	p.left.widthDelta = -4;
	p.right.widthDelta = -4;
	p.left.heightScale = 0.22f;
	p.right.heightScale = 0.22f;
	p.left.offsetY = 8;
	p.right.offsetY = 8;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid doze face parameters");
} }

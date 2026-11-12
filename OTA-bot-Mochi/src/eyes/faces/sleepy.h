/**
 * @file sleepy.h
 * @brief sleepy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sleepy_face {
using namespace face_primitives;
inline void local_eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b);


/** @brief sleepy reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)sx; (void)dx;
	float nod = (1 - cosf(phase * 0.0020943951f)) * 0.5f;
	sy -= a * 0.45f * nod; dy += a * 7 * nod;
	if (phase > 2200 && phase < 2700) {
		float wake = sinf((phase - 2200) * 0.0062831853f);
		sy += a * 0.6f * wake; dy -= a * 5 * wake;
	}
}

/** @brief sleepy local_eye_reaction_rest implementation. */
inline void local_eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t thick = eye_limit(b.h / 8, 2, 6);
	for (int16_t x = 1; x < b.w - 1; ++x) {
		int32_t t = 2 * x - b.w + 1;
		int16_t curve = int16_t(t * t * b.h / (int32_t(b.w) * b.w * 3));
		int16_t y = cfg.expression == DOZE ? b.h * 2 / 3 - curve : b.h / 2 + curve / 2;
		gfx->fillRect(b.x + x, b.y + y, 1, thick, cfg.fgColor);
	}
}

/** @brief sleepy reactionBody procedure. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)st; (void)neutral;
	uint32_t phase = elapsed % 3000;
	if (eye_reaction_amount(elapsed) < 0.8f || phase < 1300 || phase > 1800) { return false; }
	local_eye_reaction_rest(gfx, cfg, b); return true;
}

/** @brief Immutable sleepy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sleepy";
	p.left.heightScale = 0.35f;
	p.right.heightScale = 0.35f;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sleepy face parameters");
} }

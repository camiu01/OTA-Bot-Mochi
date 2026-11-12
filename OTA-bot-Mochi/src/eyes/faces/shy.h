/**
 * @file shy.h
 * @brief shy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace shy_face {
using namespace face_primitives;

/** @brief shy lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h / 4, true, cfg.bgColor);
}

/** @brief A small averted glance and downward dip return smoothly to rest. */
inline void reactionMotion(bool, uint32_t phase, float a, float &, float &, float &dx, float &dy) {
	dx += a * 5 * sinf(phase * 0.0020943951f);
	dy += a * 4 * (1 - cosf(phase * 0.0020943951f)) * 0.5f;
}

/** @brief Blush hatches live inside the cleared lower lid, so ordinary dirty bounds suffice. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st,
	const EyeBox &b, uint32_t elapsed, uint16_t) {
	if (st.blinking || b.h < 12) { return false; }
	eye_body(gfx, cfg, b); eye_pupils(gfx, cfg, st, b); lids(gfx, cfg, b);
	float blush = sinf((elapsed % 3000) * 0.0010471976f);
	uint16_t ink = eye_mix(eye_rgb(255, 112, 157), cfg.bgColor, uint8_t(blush * 80));
	int16_t length = eye_limit(b.h / 8, 1, 5), y = b.y + b.h - length - 1;
	for (uint8_t i = 0; i < 3; ++i) {
		int16_t x = b.x + b.w * (i + 2) / 6;
		gfx->drawLine(x, y + length - 1, x + length - 1, y, ink);
	}
	return true;
}

/** @brief Immutable shy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "shy";
	p.left.widthScale = 0.75f;
	p.right.widthScale = 0.75f;
	p.left.heightScale = 0.62f;
	p.right.heightScale = 0.62f;
	p.left.offsetY = 10;
	p.right.offsetY = 10;
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid shy face parameters");
} }

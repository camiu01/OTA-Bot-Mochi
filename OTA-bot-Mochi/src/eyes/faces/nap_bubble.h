/**
 * @file nap_bubble.h
 * @brief Closed dozing eyes nod while an anime snot bubble swells and shrinks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace nap_bubble_face {
/** @brief Draw a hollow bubble with one highlight, breathing with the slow nod. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float breath = 0.5f + 0.5f * sinf(ms * 0.0021f);
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(a * breath * cfg.screenH / 45);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.05f, 1 - a * 0.88f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	int16_t base = cfg.screenW / 14 + 2;
	int16_t r = int16_t(base * (0.35f + 0.65f * breath) * a) + 2;
	int16_t cx = eye_limit(pair.right.x + pair.right.w / 3, r + 1, cfg.screenW - r - 2);
	int16_t cy = eye_limit(pair.right.y + pair.right.h + r + cfg.screenH / 30, r + 1, cfg.screenH - r - 2);
	uint16_t blue = eye_mix(eye_rgb(150, 215, 255), cfg.bgColor, uint8_t(a * 100));
	gfx->drawCircle(cx, cy, r, blue);
	gfx->drawCircle(cx, cy, r - 1, blue);
	gfx->fillCircle(cx - r / 3, cy - r / 3, r / 5 + 1, blue);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("nap_bubble", scene);
static_assert(face_preset_valid(preset), "Invalid Nap Bubble face");
} }

/**
 * @file soccer.h
 * @brief Eyes track a black-and-white patched ball bouncing across the bottom.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace soccer_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The ball rolls its patches while bouncing between the sides. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float t = ms * 0.003f, p = 0.5f + 0.5f * sinf(t);
	int16_t x = int16_t(W / 6 + p * W * 2 / 3), y = H - H / 8 - int16_t(fabsf(cosf(t * 2)) * H / 5);
	pair_body(gfx, cfg, pair, a, int16_t((p - 0.5f) * a * W / 25), 0, 1, 1 - a * 0.15f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t white = tint(cfg, eye_rgb(245, 245, 245), a), black = tint(cfg, eye_rgb(30, 30, 35), a);
	int16_t r = W / 14 + 4;
	gfx->fillCircle(x, y, r, white);
	gfx->fillCircle(x, y, r / 3, black);
	for (int i = 0; i < 5; ++i) {
		float ang = i * 1.2566f + t * 2;
		gfx->fillCircle(x + int16_t(cosf(ang) * r * 0.75f), y + int16_t(sinf(ang) * r * 0.75f), r / 5 + 1, black);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("soccer", scene);
static_assert(face_preset_valid(preset), "Invalid Soccer face");
} }

/**
 * @file bowling.h
 * @brief Eyes follow a purple ball rolling into three pins, which topple outward.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace bowling_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The roll takes 1.5 seconds; the pins lean apart after the hit. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 3000;
	float roll = t / 1500.0f;
	bool hit = roll >= 1;
	int16_t base = H - H / 10, px = W - W / 5, r = W / 22 + 3, h = H / 9 + 2;
	int16_t x = hit ? px - W / 12 : int16_t(W / 8 + (px - W / 12 - W / 8) * roll);
	pair_body(gfx, cfg, pair, a, int16_t((x - W / 2) * 0.06f * a), int16_t(a * H / 50), 1, hit ? 1 - a * 0.5f : 1, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t ball = tint(cfg, eye_rgb(70, 60, 170), a), pin = tint(cfg, eye_rgb(240, 240, 235), a);
	uint16_t stripe = tint(cfg, eye_rgb(220, 50, 60), a);
	gfx->fillCircle(x, base - r, r, ball);
	for (int i = 0; i < 3; ++i) {
		int16_t cx = px + (i - 1) * W / 18, tip = hit ? int16_t((i - 1) * W / 14 * (roll - 0.7f)) : 0;
		gfx->fillTriangle(cx - 3, base, cx + 3, base, cx + tip, base - h, pin);
		gfx->drawLine(cx - 2 + tip / 2, base - h / 2, cx + 2 + tip / 2, base - h / 2, stripe);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("bowling", scene);
static_assert(face_preset_valid(preset), "Invalid Bowling face");
} }

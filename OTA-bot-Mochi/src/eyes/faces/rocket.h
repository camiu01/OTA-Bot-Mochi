/**
 * @file rocket.h
 * @brief Eyes look up as a red-and-white rocket rumbles, then launches on its flame.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace rocket_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The rocket shakes for 1.5 seconds, then accelerates upward over smoke. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 6000;
	float lift = t < 1500 ? 0 : (t - 1500) / 3500.0f;
	lift = lift > 1 ? 1 : lift;
	int16_t x = W / 2 + int16_t(lift <= 0 ? sinf(ms * 0.08f) * 1.5f : 0), y = int16_t(H - H / 6 - lift * lift * H * 1.1f);
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * lift * H / 30), 1, 1 + a * 0.1f * lift, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t hull = tint(cfg, eye_rgb(230, 230, 240), a), red = tint(cfg, eye_rgb(230, 60, 60), a);
	uint16_t flame = tint(cfg, eye_rgb(255, 170, 40), a), smoke = tint(cfg, eye_rgb(150, 150, 160), a);
	int16_t w = W / 20 + 2, h = H / 8 + 4, f = int16_t(H / 14 + sinf(ms * 0.05f) * 3) + 2;
	for (int i = 0; i < 3 && lift > 0; ++i) { gfx->fillCircle(x + (i - 1) * w * 2, H - 6 - (i % 2) * 4, w + int16_t(lift * 4), smoke); }
	gfx->fillRoundRect(x - w, y - h, w * 2, h, w / 2, hull);
	gfx->fillTriangle(x - w, y - h + 1, x + w, y - h + 1, x, y - h - w, red);
	gfx->fillTriangle(x - w, y, x - w * 2, y + w, x - w, y - w, red);
	gfx->fillTriangle(x + w, y, x + w * 2, y + w, x + w, y - w, red);
	gfx->fillTriangle(x - w / 2, y, x + w / 2, y, x, y + f, flame);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("rocket", scene);
static_assert(face_preset_valid(preset), "Invalid Rocket face");
} }

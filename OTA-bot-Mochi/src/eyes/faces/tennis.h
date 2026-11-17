/**
 * @file tennis.h
 * @brief Eyes snap left and right after a yellow ball bouncing over a low net.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace tennis_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The ball crosses the court on a sine path with fast bounces. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float p = 0.5f + 0.5f * sinf(ms * 0.005f);
	int16_t ground = H - H / 10, x = int16_t(W / 10 + p * W * 4 / 5);
	int16_t y = ground - int16_t(fabsf(cosf(ms * 0.01f)) * H / 4) - 4;
	pair_body(gfx, cfg, pair, a, int16_t((p - 0.5f) * a * W / 15), 0, 1, 1 - a * 0.25f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t court = tint(cfg, eye_rgb(80, 170, 90), a), net = tint(cfg, eye_rgb(230, 230, 230), a);
	uint16_t ball = tint(cfg, eye_rgb(210, 240, 60), a);
	gfx->drawFastHLine(0, ground, W, court);
	gfx->fillRect(W / 2 - 1, ground - H / 10, 2, H / 10, net);
	gfx->drawFastHLine(W / 2 - W / 30, ground - H / 10, W / 15, net);
	gfx->fillCircle(x, y, W / 30 + 3, ball);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("tennis", scene);
static_assert(face_preset_valid(preset), "Invalid Tennis face");
} }

/**
 * @file basketball.h
 * @brief Eyes follow an orange ball that arcs into a netted hoop and drops through.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace basketball_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each two-second shot spends 1.4 seconds in flight. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t tt = ms % 2000;
	float p = tt < 1400 ? tt / 1400.0f : 1;
	int16_t hx = W * 3 / 4, hy = H / 6, rw = W / 9 + 2, r = W / 28 + 3;
	int16_t x = int16_t(W / 6 + (hx - W / 6) * p);
	int16_t y = int16_t(H - H / 6 + (hy - H + H / 6) * p - sinf(p * 3.14159f) * H / 4);
	if (tt >= 1400) { y = hy + int16_t((tt - 1400) * H / 1600); }
	pair_body(gfx, cfg, pair, a, int16_t((x - W / 2) * 0.05f * a), int16_t((y - H / 2) * 0.05f * a), 1, 1 + a * 0.05f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t rim = tint(cfg, eye_rgb(240, 90, 40), a), net = tint(cfg, eye_rgb(220, 220, 230), a);
	uint16_t ball = tint(cfg, eye_rgb(235, 130, 40), a);
	for (int i = 0; i <= 4; ++i) { gfx->drawLine(hx - rw + i * rw / 2, hy, hx - rw / 2 + i * rw / 4, hy + H / 12, net); }
	gfx->drawFastHLine(hx - rw, hy, rw * 2, rim);
	gfx->drawFastHLine(hx - rw, hy + 1, rw * 2, rim);
	gfx->fillCircle(x, y, r, ball);
	gfx->drawFastHLine(x - r, y, r * 2, tint(cfg, eye_rgb(80, 40, 20), a));
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("basketball", scene);
static_assert(face_preset_valid(preset), "Invalid Basketball face");
} }

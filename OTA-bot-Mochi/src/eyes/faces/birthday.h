/**
 * @file birthday.h
 * @brief Smiling eyes over a pink birthday cake with three flickering candles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace birthday_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each flame flickers in height; the pair stays raised and happy. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float flicker = sinf(ms * 0.03f) * 0.5f + 0.5f;
	if (a > 0) {
		uint16_t cake = tint(cfg, eye_rgb(250, 200, 220), a), cream = tint(cfg, eye_rgb(255, 250, 240), a);
		uint16_t candle = tint(cfg, eye_rgb(110, 180, 255), a), flame = tint(cfg, eye_rgb(255, 180, 40), a);
		int16_t w = W / 3, h = H / 9, x = W / 2 - w / 2, y = H - h - 4, ch = H / 14;
		gfx->fillRect(x, y, w, h, cake);
		gfx->fillRect(x, y, w, 3, cream);
		for (int i = 0; i < 3; ++i) {
			int16_t cx = x + w * (i + 1) / 4;
			gfx->fillRect(cx - 1, y - ch, 3, ch, candle);
			gfx->fillEllipse(cx, y - ch - 4, W / 80 + 2, 4 + int16_t(flicker * 3) + (i == 1 ? 1 : 0), flame);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * H / 50), 1, 1 - a * 0.65f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("birthday", scene);
static_assert(face_preset_valid(preset), "Invalid Birthday face");
} }

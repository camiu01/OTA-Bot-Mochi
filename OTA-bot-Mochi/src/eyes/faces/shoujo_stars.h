/**
 * @file shoujo_stars.h
 * @brief Enlarged dreamy eyes surrounded by twinkling gold and pink shoujo stars.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace shoujo_stars_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each four-point star pulses with its own phase; no pupils are added. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, 0, 0, 1 + a * 0.15f, 1 + a * 0.2f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t gold = tint(cfg, eye_rgb(255, 235, 150), a), pink = tint(cfg, eye_rgb(255, 170, 220), a);
	for (int i = 0; i < 6; ++i) {
		int16_t x = int16_t((i * 67 + 23) % W), y = int16_t((i * 41 + 13) % (H / 3)) + (i & 1 ? H * 2 / 3 : 0);
		int16_t s = int16_t((0.5f + 0.5f * sinf(ms * 0.006f + i * 1.3f)) * (W / 25)) + 1;
		uint16_t c = i & 1 ? gold : pink;
		gfx->drawLine(x - s, y, x + s, y, c);
		gfx->drawLine(x, y - s, x, y + s, c);
		gfx->drawLine(x - s / 2, y - s / 2, x + s / 2, y + s / 2, c);
		gfx->drawLine(x - s / 2, y + s / 2, x + s / 2, y - s / 2, c);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("shoujo_stars", scene);
static_assert(face_preset_valid(preset), "Invalid Shoujo Stars face");
} }

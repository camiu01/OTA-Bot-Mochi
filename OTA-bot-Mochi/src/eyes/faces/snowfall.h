/**
 * @file snowfall.h
 * @brief Calm, slightly lowered eyes under drifting six-armed snowflakes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace snowfall_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Flakes fall at different phases and sway sideways; the eyes soften. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * H / 60), 1, 1 - a * 0.3f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t snow = tint(cfg, eye_rgb(220, 235, 255), a);
	int16_t s = W / 70 + 1;
	for (int i = 0; i < 9; ++i) {
		int16_t x = int16_t((i * 53 + 17) % W + sinf(ms * 0.002f + i) * W / 30);
		int16_t y = int16_t((ms / 25 + i * H / 9) % H);
		gfx->drawLine(x - s, y, x + s, y, snow);
		gfx->drawLine(x, y - s, x, y + s, snow);
		gfx->drawLine(x - s, y - s, x + s, y + s, snow);
		gfx->drawLine(x - s, y + s, x + s, y - s, snow);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("snowfall", scene);
static_assert(face_preset_valid(preset), "Invalid Snowfall face");
} }

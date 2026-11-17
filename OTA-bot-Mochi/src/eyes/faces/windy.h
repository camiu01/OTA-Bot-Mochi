/**
 * @file windy.h
 * @brief Eyes squint and lean into gusts while wind curls and a leaf tumble past.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace windy_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Gust strength drives the lean and squint; wind strokes stream across. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float gust = 0.5f + 0.5f * sinf(ms * 0.003f);
	pair_body(gfx, cfg, pair, a, -int16_t(a * gust * W / 30), 0, 1 + a * 0.1f, 1 - a * (0.4f + 0.3f * gust), cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t wind = tint(cfg, eye_rgb(185, 215, 235), a), leaf = tint(cfg, eye_rgb(110, 200, 90), a);
	for (int i = 0; i < 4; ++i) {
		int16_t y = H / 6 + i * H / 5;
		int16_t x = int16_t((ms / 5 + i * W / 3) % (W + W / 2)) - W / 4;
		gfx->drawFastHLine(x, y, W / 4, wind);
		gfx->drawFastHLine(x, y + 1, W / 4, wind);
		gfx->drawCircle(x + W / 4, y - 4, 4, wind);
	}
	int16_t lx = int16_t((ms / 4) % W), ly = H / 3 + int16_t(sinf(ms * 0.01f) * H / 10);
	gfx->fillEllipse(lx, ly, W / 22 + 3, W / 45 + 2, leaf);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("windy", scene);
static_assert(face_preset_valid(preset), "Invalid Windy face");
} }

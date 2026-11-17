/**
 * @file pumpkin.h
 * @brief Warm candle-lit eyes inside a ribbed orange pumpkin outline with a leafy stem.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace pumpkin_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Outline an ellipse with 24 straight segments using only portable primitives. */
inline void oval(Arduino_GFX *gfx, int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t c) {
	for (int i = 0; i < 24; ++i) {
		float t0 = i * 0.2618f, t1 = (i + 1) * 0.2618f;
		gfx->drawLine(cx + int16_t(cosf(t0) * rx), cy + int16_t(sinf(t0) * ry),
			cx + int16_t(cosf(t1) * rx), cy + int16_t(sinf(t1) * ry), c);
	}
}
/** @brief The candle glow flickers the eye height; the leaf trembles. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float flick = 0.5f + 0.5f * sinf(ms * 0.02f);
	if (a > 0) {
		uint16_t orange = tint(cfg, eye_rgb(240, 130, 30), a), stem = tint(cfg, eye_rgb(80, 150, 60), a);
		int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 2, rx = W * 9 / 20, ry = H * 2 / 5;
		oval(gfx, cx, cy, rx, ry, orange);
		oval(gfx, cx, cy, rx - 1, ry - 1, orange);
		oval(gfx, cx, cy, rx / 2, ry, orange);
		gfx->fillRect(cx - 3, cy - ry - H / 14, 6, H / 14, stem);
		gfx->fillEllipse(cx + W / 20, cy - ry - H / 20 + int16_t(flick * 2), W / 25, H / 60 + 2, stem);
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1, 1 - a * (0.3f + 0.1f * flick), eye_rgb(255, 170, 40));
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("pumpkin", scene);
static_assert(face_preset_valid(preset), "Invalid Pumpkin face");
} }

/**
 * @file heatwave.h
 * @brief Wilting, wobbling eyes under a rotating sun with shimmering heat lines.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace heatwave_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The sun rays turn while three sine lines shimmer below the sagging pair. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float wob = sinf(ms * 0.006f);
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * H / 25), 1 + a * (0.15f + 0.05f * wob), 1 - a * (0.6f + 0.1f * wob), cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t sun = tint(cfg, eye_rgb(255, 200, 40), a), heat = tint(cfg, eye_rgb(255, 120, 40), a);
	int16_t r = W / 14 + 2, cx = W - r * 2, cy = r * 2;
	gfx->fillCircle(cx, cy, r, sun);
	for (const auto &b : {pair.left, pair.right}) {
		int16_t drop = int16_t((ms / 20 + (b.left ? 0 : 37)) % (H / 10 + 1));
		gfx->fillCircle(b.x + b.w * 3 / 4, b.y + b.h + H / 30 + drop, W / 60 + 2, tint(cfg, eye_rgb(120, 190, 255), a));
	}
	for (int i = 0; i < 8; ++i) {
		float t = i * 0.785f + ms * 0.0015f;
		gfx->drawLine(cx + int16_t(cosf(t) * r * 1.3f), cy + int16_t(sinf(t) * r * 1.3f),
			cx + int16_t(cosf(t) * r * 1.8f), cy + int16_t(sinf(t) * r * 1.8f), sun);
	}
	int16_t y = pair.left.y + pair.left.h + H / 10;
	for (int k = 0; k < 3; ++k) {
		float p = ms * 0.01f + k * 2;
		for (int16_t x = W / 6; x < W * 5 / 6; x += 4) {
			gfx->drawLine(x, y + k * 6 + int16_t(sinf(x * 0.15f + p) * 3),
				x + 4, y + k * 6 + int16_t(sinf((x + 4) * 0.15f + p) * 3), heat);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("heatwave", scene);
static_assert(face_preset_valid(preset), "Invalid Heatwave face");
} }

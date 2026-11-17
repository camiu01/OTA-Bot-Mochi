/**
 * @file determined.h
 * @brief Narrowed eyes under set diagonal brows, with speed lines streaming behind.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace determined_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Speed lines race right to left at different lengths. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	if (a > 0) {
		uint16_t speed = tint(cfg, eye_rgb(120, 160, 220), a);
		for (int i = 0; i < 8; ++i) {
			int16_t y = int16_t((i * 37 + 11) % H), len = W / 5 + (i % 3) * W / 10;
			int16_t x = W - int16_t((ms / 2 + i * 53) % (W + len));
			gfx->drawFastHLine(x, y, len, speed);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 + a * 0.05f, 1 - a * 0.45f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t ink = tint(cfg, cfg.fgColor, a);
	for (auto b : {pair.left, pair.right}) {
		int16_t y = b.y + b.h / 4 - H / 25, inner = b.left ? b.x + b.w : b.x, outer = b.left ? b.x : b.x + b.w;
		for (int k = 0; k < W / 60 + 2; ++k) { gfx->drawLine(outer, y - H / 30 + k, inner, y + H / 50 + k, ink); }
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("determined", scene);
static_assert(face_preset_valid(preset), "Invalid Determined face");
} }

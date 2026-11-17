/**
 * @file hypnotized.h
 * @brief Eyes sway after a swinging gold pendulum inside spreading violet rings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace hypnotized_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Three rings expand around each eye while the pendulum swings. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float swing = sinf(ms * 0.004f);
	int16_t px = W / 2 + int16_t(swing * W / 5), py = H / 2 + H / 4;
	if (a > 0) {
		uint16_t ring = tint(cfg, eye_rgb(170, 90, 230), a), coin = tint(cfg, eye_rgb(255, 200, 60), a);
		for (auto b : {pair.left, pair.right}) {
			int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, base = (b.w > b.h ? b.w : b.h) / 2;
			for (int k = 1; k <= 3; ++k) {
				gfx->drawCircle(cx, cy, base + k * W / 40 + int16_t((ms / 40) % (W / 40 + 1)), ring);
			}
		}
		gfx->drawLine(W / 2, 0, px, py, coin);
		gfx->fillCircle(px, py, W / 30 + 2, coin);
	}
	pair_body(gfx, cfg, pair, a, int16_t(swing * a * W / 40), 0, 1, 1 - a * 0.2f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("hypnotized", scene);
static_assert(face_preset_valid(preset), "Invalid Hypnotized face");
} }

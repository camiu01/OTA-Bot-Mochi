/**
 * @file rainbow.h
 * @brief Smiling raised eyes while a five-band rainbow arc draws itself behind them.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace rainbow_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The arc grows during the first half of the hold, then stays complete. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	if (a > 0) {
		constexpr uint16_t bands[5] = {eye_rgb(255, 70, 70), eye_rgb(255, 165, 40), eye_rgb(255, 230, 60),
			eye_rgb(80, 210, 100), eye_rgb(80, 140, 255)};
		float grow = (ms % 6000) / 2600.0f;
		int steps = grow > 1 ? 24 : 1 + int(grow * 23);
		int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 3, r0 = W * 2 / 5;
		for (int band = 0; band < 5; ++band) {
			uint16_t c = tint(cfg, bands[band], a);
			int16_t r = r0 - band * (W / 50 + 1);
			for (int i = 0; i < steps; ++i) {
				float t0 = 3.14159f * (1 - i / 24.0f), t1 = 3.14159f * (1 - (i + 1) / 24.0f);
				gfx->drawLine(cx + int16_t(cosf(t0) * r), cy - int16_t(sinf(t0) * r),
					cx + int16_t(cosf(t1) * r), cy - int16_t(sinf(t1) * r), c);
			}
		}
	}
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * H / 50), 1, 1 - a * 0.6f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("rainbow", scene);
static_assert(face_preset_valid(preset), "Invalid Rainbow face");
} }

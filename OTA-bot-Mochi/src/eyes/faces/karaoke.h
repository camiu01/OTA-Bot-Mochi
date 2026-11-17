/**
 * @file karaoke.h
 * @brief Closed, happily singing eyes sway behind a microphone as notes float up.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace karaoke_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Three notes rise at staggered phases; the pair sways side to side. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float sway = sinf(ms * 0.004f);
	if (a > 0) {
		uint16_t mic = tint(cfg, eye_rgb(170, 175, 185), a), handle = tint(cfg, eye_rgb(60, 60, 70), a);
		uint16_t note = tint(cfg, eye_rgb(255, 120, 200), a);
		int16_t cx = W / 2, y = H - H / 4, r = W / 22 + 3;
		gfx->fillCircle(cx, y, r, mic);
		gfx->fillRect(cx - r / 3, y + r, r * 2 / 3 + 1, H / 6, handle);
		for (int i = 0; i < 3; ++i) {
			int16_t ny = H / 2 - int16_t((ms / 12 + i * 40) % (H / 2));
			int16_t nx = cx + (i - 1) * W / 4 + int16_t(sinf(ms * 0.005f + i) * W / 30);
			gfx->fillCircle(nx, ny, W / 70 + 2, note);
			gfx->drawFastVLine(nx + W / 70 + 1, ny - H / 20, H / 20, note);
		}
	}
	pair_body(gfx, cfg, pair, a, int16_t(sway * a * W / 40), 0, 1, 1 - a * 0.8f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("karaoke", scene);
static_assert(face_preset_valid(preset), "Invalid Karaoke face");
} }

/**
 * @file ufo.h
 * @brief A hovering saucer shines a green tractor beam that gently lifts the eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace ufo_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Three hull lights chase every 200 ms while the saucer drifts. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float hover = sinf(ms * 0.004f);
	int16_t cx = W / 2 + int16_t(hover * W / 12), cy = H / 8 + 4;
	if (a > 0) {
		uint16_t beam = tint(cfg, eye_rgb(150, 255, 170), a * 0.55f), hull = tint(cfg, eye_rgb(170, 180, 200), a);
		uint16_t dome = tint(cfg, eye_rgb(120, 220, 255), a);
		for (int16_t y = cy + 4 + int16_t((ms / 40) % 6); y < H - 4; y += 6) {
			int16_t half = int16_t((y - cy) * W / 5 / (H - cy));
			gfx->drawFastHLine(cx - half, y, half * 2, beam);
		}
		gfx->fillEllipse(cx, cy - H / 40, W / 14, H / 30 + 2, dome);
		gfx->fillEllipse(cx, cy, W / 6, H / 40 + 2, hull);
		for (int i = 0; i < 3; ++i) {
			bool on = (ms / 200 + i) % 3 == 0;
			gfx->fillCircle(cx + (i - 1) * W / 12, cy, 2, tint(cfg, on ? eye_rgb(255, 230, 80) : eye_rgb(90, 90, 110), a));
		}
	}
	pair_body(gfx, cfg, pair, a, int16_t(hover * a * W / 40), -int16_t(a * (2 + hover * 2) * H / 60), 1 - a * 0.05f, 1 + a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("ufo", scene);
static_assert(face_preset_valid(preset), "Invalid Ufo face");
} }

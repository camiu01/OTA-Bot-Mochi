/**
 * @file ramen.h
 * @brief Happy eyes over a steaming red ramen bowl as chopsticks lift the noodles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace ramen_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Noodles stretch up and down with the chopsticks while steam rises. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	if (a > 0) {
		float lift = 0.5f + 0.5f * sinf(ms * 0.004f);
		uint16_t bowl = tint(cfg, eye_rgb(205, 60, 50), a), noodle = tint(cfg, eye_rgb(250, 220, 120), a);
		uint16_t stick = tint(cfg, eye_rgb(170, 120, 70), a), steam = tint(cfg, eye_rgb(200, 205, 215), a);
		int16_t r = W / 6, cx = W / 2, cy = H - H / 5;
		gfx->fillRoundRect(cx - r, cy, r * 2, r * 2 / 3, r / 3, bowl);
		int16_t top = cy - int16_t(lift * H / 8) - H / 30;
		for (int i = -1; i <= 1; ++i) {
			gfx->drawLine(cx + i * 4, cy, cx + i * 4 + int16_t(sinf(ms * 0.02f + i) * 2), top, noodle);
		}
		gfx->drawLine(cx - 6, top - 2, cx + r, top - H / 10, stick);
		gfx->drawLine(cx + 6, top - 2, cx + r + 6, top - H / 10, stick);
		for (int i = 0; i < 2; ++i) {
			int16_t x = cx - r / 2 + i * r, y = cy - int16_t((ms / 20 + i * 30) % (H / 8));
			gfx->drawLine(x, y, x + int16_t(sinf(ms * 0.01f + i) * 3), y - H / 25, steam);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * H / 50), 1, 1 - a * 0.6f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("ramen", scene);
static_assert(face_preset_valid(preset), "Invalid Ramen face");
} }

/**
 * @file cookie.h
 * @brief Munching eyes as bites disappear one by one from a chocolate-chip cookie.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace cookie_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief One bite is taken every 1.2 seconds while crumbs fall below. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint8_t bites = uint8_t((ms % 6000) / 1200);
	if (a > 0) {
		uint16_t dough = tint(cfg, eye_rgb(210, 150, 80), a), chip = tint(cfg, eye_rgb(80, 45, 30), a);
		int16_t r = W / 9 + 2, cx = W / 2, cy = H - r - H / 14;
		gfx->fillCircle(cx, cy, r, dough);
		gfx->fillCircle(cx - r / 3, cy - r / 4, 2, chip);
		gfx->fillCircle(cx + r / 3, cy, 2, chip);
		gfx->fillCircle(cx - r / 5, cy + r / 2, 2, chip);
		for (uint8_t i = 0; i < bites; ++i) { gfx->fillCircle(cx + r - i * r / 3, cy - r + i * r / 4, r / 3, cfg.bgColor); }
		for (int i = 0; i < 3; ++i) {
			int16_t y = cy + r + int16_t((ms / 15 + i * 9) % (H / 10 + 1));
			gfx->fillRect(cx + r / 2 - i * 4, y, 2, 2, dough);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(sinf(ms * 0.02f) * a * 1.5f), 1, 1 - a * 0.45f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("cookie", scene);
static_assert(face_preset_valid(preset), "Invalid Cookie face");
} }

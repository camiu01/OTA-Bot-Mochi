/**
 * @file steam_blush.h
 * @brief Overheated flustered eyes squeeze shut under rising steam puffs and full blush.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace steam_blush_face {
/** @brief Two puffs per side climb from the temples and shrink as they rise. */
inline void steam(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t ms, float a) {
	uint16_t white = eye_mix(eye_rgb(225, 228, 238), cfg.bgColor, uint8_t(a * 100));
	int16_t rise = cfg.screenH / 6 + 4, r0 = cfg.screenW / 18 + 3;
	for (int i = 0; i < 2; ++i) {
		int16_t climb = int16_t((ms / 14 + i * rise / 2) % rise);
		int16_t r = r0 - int16_t(climb * (r0 - 1) / rise);
		int16_t x = b.left ? b.x - r0 : b.x + b.w + r0;
		x += int16_t(sinf((ms + i * 400) * 0.006f) * r0 / 2);
		int16_t y = eye_limit(b.y - climb, r + 1, cfg.screenH - r - 2);
		gfx->fillCircle(eye_limit(x, r + 1, cfg.screenW - r - 2), y, r, white);
	}
}
/** @brief Tremble the squeezed pair over pink cheek ovals. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	int16_t shiver = int16_t(sinf(ms * 0.06f) * a * 1.5f);
	for (auto b : {pair.left, pair.right}) {
		if (a > 0) {
			uint16_t pink = eye_mix(eye_rgb(255, 120, 150), cfg.bgColor, uint8_t(a * 100));
			gfx->fillEllipse(b.x + b.w / 2, b.y + b.h * 3 / 4, b.w / 2 + 2, b.h / 6 + 2, pink);
			steam(gfx, cfg, b, ms, a);
		}
		b.x += shiver;
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.06f, 1 - a * 0.8f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("steam_blush", scene);
static_assert(face_preset_valid(preset), "Invalid Steam Blush face");
} }

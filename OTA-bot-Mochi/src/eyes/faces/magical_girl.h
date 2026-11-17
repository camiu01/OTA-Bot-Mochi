/**
 * @file magical_girl.h
 * @brief Pink transformation ribbons orbit real eyes beneath a small tiara.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace magical_girl_face {
/** @brief Weave two soft ribbons below the eye pair, never adding pupils to solid styles. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t pink = eye_mix(eye_rgb(255, 130, 210), cfg.bgColor, uint8_t(a * 100));
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 + a * 0.07f, eye_rgb(255, 219, 245));
		if (a <= 0) { continue; }
		int16_t cy = b.y + b.h + cfg.screenH / 25, cx = b.x + b.w / 2;
		for (int x = -b.w / 2; x < b.w / 2; ++x) {
			int16_t y = int16_t(sinf(x * 0.1f + ms * 0.003f) * cfg.screenH / 65);
			gfx->fillRect(cx + x, cy + y, 1, 2, pink);
		}
	}
	if (a > 0) {
		int16_t x = cfg.screenW / 2, y = pair.left.y - cfg.screenH / 14, r = cfg.screenW / 30 + 1;
		gfx->drawLine(x - r * 2, y + r, x, y - r, pink);
		gfx->drawLine(x, y - r, x + r * 2, y + r, pink);
		gfx->fillTriangle(x, y - r / 2, x - r / 2, y, x + r / 2, y, pink);
		gfx->fillTriangle(x - r / 2, y, x + r / 2, y, x, y + r / 2, pink);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("magical_girl", scene);
static_assert(face_preset_valid(preset), "Invalid Magical Girl face");
} }

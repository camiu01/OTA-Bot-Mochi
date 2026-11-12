/**
 * @file spirit_seal.h
 * @brief Floating ofuda paper charms frame watchful violet eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace spirit_seal_face {
/** @brief Suspend two marked paper strips above the eyes with slow independent sway. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.15f, eye_rgb(205, 170, 255));
		if (a <= 0) { continue; }
		int16_t w = cfg.screenW / 18 + 2, h = cfg.screenH / 9;
		int16_t x = b.x + b.w / 2 - w / 2 + int16_t(sinf(ms * 0.002f + (b.left ? 0 : 2)) * w / 3);
		int16_t y = b.y - h - cfg.screenH / 30;
		uint16_t paper = eye_mix(eye_rgb(255, 237, 193), cfg.bgColor, uint8_t(a * 100));
		uint16_t red = eye_mix(eye_rgb(190, 55, 80), cfg.bgColor, uint8_t(a * 100));
		gfx->fillRect(x, y, w, h, paper);
		gfx->drawLine(x + w / 2, y + h / 6, x + w / 2, y + h * 5 / 6, red);
		for (int i = 1; i < 4; ++i) {
			gfx->drawLine(x + 2, y + h * i / 4, x + w - 3, y + h * i / 4 - 2, red);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("spirit_seal", scene);
static_assert(face_preset_valid(preset), "Invalid Spirit Seal face");
} }

/**
 * @file chibi_joy.h
 * @brief Bouncing happy chevrons with sparse pastel confetti, no mouth or extra pupils.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace chibi_joy_face {
/** @brief Morph the real pair into smiling upturned strokes and back to the selected style. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.y -= int16_t((0.5f + 0.5f * sinf(ms * 0.008f)) * a * cfg.screenH / 50);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.9f, cfg.fgColor, 1 - a);
		if (a <= 0) { continue; }
		uint16_t ink = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 100));
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, h = cfg.screenH / 24;
		for (int k = 0; k < cfg.screenW / 90 + 1; ++k) {
			gfx->drawLine(b.x + b.w / 6, cy + h / 2 + k, cx, cy - h / 2 + k, ink);
			gfx->drawLine(cx, cy - h / 2 + k, b.x + b.w * 5 / 6, cy + h / 2 + k, ink);
		}
		for (int i = 0; i < 3; ++i) {
			int16_t x = b.x + b.w * i / 2;
			int16_t y = b.y - cfg.screenH / 20 - int16_t((ms / 35 + i * 13) % (cfg.screenH / 10));
			uint16_t color = eye_mix(i & 1 ? eye_rgb(255, 160, 205) : eye_rgb(255, 221, 115), cfg.bgColor, uint8_t(a * 100));
			gfx->fillRect(x, y, cfg.screenW / 90 + 1, cfg.screenH / 70 + 1, color);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("chibi_joy", scene);
static_assert(face_preset_valid(preset), "Invalid Chibi Joy face");
} }

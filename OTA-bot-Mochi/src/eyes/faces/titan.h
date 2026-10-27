/**
 * @file titan.h
 * @brief Expanding amber eyes surrounded by original red energy ribs.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace titan_face {
/** @brief Pulse angular energy ribs without recreating a character's face. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	float pulse = 0.7f + 0.2f * sinf((elapsed % 6000) * 0.0041887902f);
	uint16_t ink = eye_mix(eye_rgb(225, 80, 65), cfg.bgColor, uint8_t(a * pulse * 100));
	for (uint8_t i = 0; i < 5; ++i) {
		int16_t y = cfg.screenH / 5 + i * cfg.screenH * 3 / 25;
		int16_t inset = cfg.screenW / 20 + i * cfg.screenW / 100;
		for (int16_t side : {-1, 1}) {
			int16_t x = side < 0 ? inset : cfg.screenW - inset;
			gfx->drawLine(x, y, x - side * cfg.screenW / 16, y + cfg.screenH / 25, ink);
			gfx->drawLine(x, y, x, y + cfg.screenH / 18, ink);
		}
	}
}
/** @brief Grow with a slow breathing cycle. */
inline void motion(bool, uint32_t phase, float a, float &sx, float &sy, float &, float &) {
	float breath = 0.22f + 0.04f * sinf(phase * 0.0041887902f);
	sx += a * breath; sy += a * breath;
}
/** @brief Register the original titan-themed performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "titan"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(245, 170, 100); p.scenery = scenery; p.performanceMotion = motion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid titan face");
} }

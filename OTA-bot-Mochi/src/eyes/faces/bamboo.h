/**
 * @file bamboo.h
 * @brief Original bamboo grove around pink, gently bobbing eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_ambient_backgrounds.h"
namespace face_defs { namespace bamboo_face {
/** @brief Sway two segmented stalks at the sides of the eye pair. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	int16_t width = cfg.screenW / 32 + 2, height = cfg.screenH / 2;
	uint16_t green = eye_mix(eye_rgb(125, 190, 85), cfg.bgColor, uint8_t(a * 100));
	uint16_t joint = eye_mix(eye_rgb(205, 220, 125), cfg.bgColor, uint8_t(a * 100));
	for (int16_t side : {-1, 1}) {
		int16_t x = cfg.screenW / 2 + side * cfg.screenW * 2 / 5;
		x += int16_t(sinf((elapsed % 6000) * 0.0020943951f) * width / 3);
		int16_t top = cfg.screenH / 4;
		gfx->fillRoundRect(x - width / 2, top, width, height, width / 3, green);
		for (uint8_t i = 1; i < 4; ++i) {
			int16_t y = top + i * height / 4;
			gfx->fillRect(x - width / 2, y, width, 2, joint);
			gfx->fillTriangle(x, y, x - side * width * 2, y - width, x - side * width, y, green);
		}
	}
}
/** @brief Bob in the grove without changing the saved eye shape. */
inline void motion(bool, uint32_t phase, float a, float &, float &, float &, float &dy) {
	dy = a * 4 * sinf(phase * 0.0041887902f);
}
/** @brief Register the bamboo performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "bamboo"; p.background = face_backgrounds::grove;
	p.bounds = face_backgrounds::full_bounds; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(245, 140, 200); p.scenery = scenery; p.performanceMotion = motion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid bamboo face");
} }

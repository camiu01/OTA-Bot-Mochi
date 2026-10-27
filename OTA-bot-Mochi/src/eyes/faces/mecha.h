/**
 * @file mecha.h
 * @brief Original robot visor with antennae and a slow scanner.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_backgrounds.h"
namespace face_defs { namespace mecha_face {
/** @brief Draw original armor outlines, without character artwork or emblems. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	int16_t w = cfg.screenW, h = cfg.screenH;
	uint16_t armor = eye_mix(eye_rgb(115, 150, 180), cfg.bgColor, uint8_t(a * 100));
	for (int16_t side : {-1, 1}) {
		gfx->drawLine(w / 2 + side * w / 16, h / 3, w / 2 + side * w / 4, h / 6, armor);
		gfx->drawLine(w / 2 + side * w / 3, h / 3, w / 2 + side * w * 5 / 12, h / 2, armor);
		gfx->drawLine(w / 2 + side * w * 5 / 12, h / 2, w / 2 + side * w / 3, h * 2 / 3, armor);
	}
	int16_t x = w / 8 + int32_t(elapsed % 3000) * (w * 3 / 4) / 3000;
	uint16_t scanner = eye_mix(eye_rgb(100, 230, 220), cfg.bgColor, uint8_t(a * 100));
	gfx->fillRect(x, h / 4, w / 100 + 1, h / 20, scanner);
}
/** @brief Compress the eyes into a broad visor. */
inline void motion(bool, uint32_t, float a, float &sx, float &sy, float &, float &) {
	sx += a * 0.12f; sy -= a * 0.65f;
}
/** @brief Register the mecha performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "mecha"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(245, 210, 85); p.scenery = scenery; p.performanceMotion = motion;
	p.background = face_backgrounds::armor; p.bounds = face_backgrounds::full_bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid mecha face");
} }

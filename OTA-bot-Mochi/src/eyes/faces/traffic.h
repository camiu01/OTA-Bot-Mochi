/**
 * @file traffic.h
 * @brief Decorative red/amber/green lamps, not a real traffic signal.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_ambient_backgrounds.h"
namespace face_defs { namespace traffic_face {
/** @brief Fade sequential colors through the shared reversible loop. */
inline uint16_t color(const EyeConfig &, bool, uint32_t elapsed) {
	static constexpr uint16_t inks[] = {eye_rgb(245, 90, 80),
		eye_rgb(255, 190, 70), eye_rgb(90, 230, 150)};
	uint32_t phase = (elapsed % 6000) / 1000;
	return inks[phase < 2 ? 0 : (phase < 3 ? 1 : 2)];
}
/** @brief Draw a rounded lamp hood and a stationary dot grid. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	uint16_t ink = eye_mix(eye_rgb(145, 155, 175), cfg.bgColor, uint8_t(a * 100));
	for (const EyeBox &b : {layout.left, layout.right}) {
		int16_t pad = cfg.screenW / 60 + 1;
		gfx->fillRoundRect(b.x - pad, b.y - pad, b.w + 2 * pad, b.h + 2 * pad, b.radius, ink);
	}
	(void)elapsed;
}
/** @brief Fade a small lens grid into each colored body. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	int16_t step = cfg.screenW / 60 + 2;
	for (const EyeBox &b : {layout.left, layout.right}) {
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye_body(gfx, ink, b);
		uint16_t cut = eye_mix(cfg.bgColor, ink.fgColor, uint8_t(a * 100));
		for (int16_t y = b.y + step; y < b.y + b.h - step; y += step) {
			for (int16_t x = b.x + step; x < b.x + b.w - step; x += step) {
				gfx->fillRect(x, y, 1, 1, cut);
			}
		}
	}
	return true;
}
/** @brief Register the decorative lamp performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "traffic"; p.background = face_backgrounds::city;
	p.bounds = face_backgrounds::full_bounds; p.performance = p.fullScene = true;
	p.performanceColor = color; p.scenery = scenery; p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid traffic face");
} }

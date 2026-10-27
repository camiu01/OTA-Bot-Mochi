/**
 * @file mistake.h
 * @brief mistake definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace mistake_face {
using namespace face_primitives;
inline void local_eye_extra_banner(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);


/** @brief mistake performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = MISTAKE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * 0.6f; dy = -a * 34; dx = a * 2 * sinf(phase * 0.05f);
}

/** @brief mistake local_eye_extra_banner implementation. */
inline void local_eye_extra_banner(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	bool glitch = cfg.expression == MISTAKE;
	const char *text = glitch ? "OTA BOT!" : "GO OTA BOT!";
	int16_t cx = cfg.screenW / 2, y = cfg.screenH * 60 / 100;
	uint32_t step = (elapsed % 6000) / 90;
	uint16_t ink = eye_mix(eye_performance_ink(cfg, true, elapsed), cfg.bgColor, uint8_t(a * 100));
	if (!glitch) {
		eye_extra_text(gfx, cfg, text, cx, y, ink);
		int16_t half = int16_t(cfg.screenW * 3 / 8 * a);
		gfx->fillRect(cx - half, y + cfg.screenH / 8, half * 2, 3, ink);
		return;
	}
	int16_t shift = int16_t((step * 7) % 9) - 4;
	eye_extra_text(gfx, cfg, text, cx + shift * 2, y, eye_mix(eye_rgb(245, 70, 90), cfg.bgColor, uint8_t(a * 60)));
	eye_extra_text(gfx, cfg, text, cx - shift, y, ink);
	for (uint8_t i = 0; i < 3; ++i) {
		int16_t bar = int16_t(y + ((step * 5 + i * 7) % 12) * cfg.screenH / 80);
		gfx->fillRect(0, bar, cfg.screenW, 2, cfg.bgColor);
	}
}

/** @brief mistake scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	(void)layout;
	local_eye_extra_banner(gfx, cfg, elapsed, a);
}

/** @brief Immutable mistake face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "mistake";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(235, 235, 235);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid mistake face parameters");
} }

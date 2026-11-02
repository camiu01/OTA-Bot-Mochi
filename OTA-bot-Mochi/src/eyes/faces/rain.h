/**
 * @file rain.h
 * @brief rain definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace rain_face {
using namespace face_primitives;
inline void local_eye_performance_rain(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);


/** @brief rain performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = RAIN; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * 0.28f; dy = a * 5;
}

/** @brief rain local_eye_performance_rain implementation. */
inline void local_eye_performance_rain(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	uint32_t phase = elapsed % 6000;
	if (phase < 600) { return; }
	uint16_t ink = eye_mix(eye_rgb(80, 180, 245), cfg.bgColor, uint8_t(a * 85));
	for (uint8_t i = 0; i < 12; ++i) {
		if (phase - 600 < i * 80u) { continue; }
		uint32_t travel = phase - 600 - i * 80u;
		uint32_t event = elapsed / 6000 * 5 + travel / 1700;
		uint32_t age = travel % 1700;
		uint32_t period = 1100u + uint32_t(face_random(event, 131u + i * 71u) * 500);
		if (age >= period) { continue; }
		int16_t tail = int16_t((cfg.screenH / 35 + 2) * (0.7f + face_random(event, 337u + i * 59u)));
		int16_t x = int16_t(2 + face_random(event, 173u + i * 113u) * (cfg.screenW - 5));
		int16_t head = int32_t(age) * (cfg.screenH + tail) / period;
		int16_t top = eye_limit(head - tail, 0, cfg.screenH - 1);
		int16_t bottom = eye_limit(head, top, cfg.screenH - 1);
		gfx->drawLine(x, top, x, bottom, ink);
	}
}

/** @brief rain scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	(void)layout;
	local_eye_performance_rain(gfx, cfg, elapsed, a);
}

/** @brief Immutable rain face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "rain";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(80, 175, 245);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid rain face parameters");
} }

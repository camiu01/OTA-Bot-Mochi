/**
 * @file comet.h
 * @brief Low curious eyes follow a sequence of gently fading shooting stars.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace comet_face {
/** @brief Lower the eyes and let their pair sway after the passing lights. */
inline void motion(bool, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	sx -= a * 0.35f; sy -= a * 0.25f;
	dy += a * 38; dx += a * 6 * sinf(phase * 0.0041887902f);
}

/** @brief Traverse the free sky with three staggered, tapered comet tails. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	for (uint8_t i = 0; i < 3; ++i) {
		uint32_t travel = elapsed + i * 670u;
		float t = (travel % 2000) / 2000.0f;
		float lane = face_random(travel / 2000, 67u + i * 113u, cfg.variationSeed);
		float visibility = sinf(t * 3.14159265f);
		int16_t x = int16_t(cfg.screenW * (0.12f + 0.75f * t));
		int16_t y = int16_t(cfg.screenH * (0.09f + 0.20f * lane + 0.12f * t));
		for (uint8_t k = 1; k <= 6; ++k) {
			uint16_t ink = eye_mix(eye_rgb(124, 180, 245), cfg.bgColor,
				uint8_t(a * visibility * (7 - k) * 10));
			gfx->drawLine(x - k * cfg.screenW / 75, y - k * cfg.screenH / 160,
				x - (k - 1) * cfg.screenW / 75, y - (k - 1) * cfg.screenH / 160, ink);
		}
		uint16_t head = eye_mix(eye_rgb(225, 235, 252), cfg.bgColor, uint8_t(a * visibility * 100));
		face_primitives::sparkle(gfx, x, y, cfg.screenW / 35, head);
	}
}

/** @brief Register a moving sky distinct from the stationary stargazing backdrop. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "comet"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(154, 196, 246);
	p.performanceMotion = motion; p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid comet face");
} }

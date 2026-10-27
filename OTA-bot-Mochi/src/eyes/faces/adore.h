/**
 * @file adore.h
 * @brief adore definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace adore_face {
using namespace face_primitives;
inline void local_eye_extra_hearts(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);


/** @brief adore performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = ADORE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += a * 0.12f; sy += a * 0.08f * (1 + wave);
}

/** @brief adore local_eye_extra_hearts implementation. */
inline void local_eye_extra_hearts(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	uint16_t ink = eye_mix(eye_rgb(255, 110, 160), cfg.bgColor, uint8_t(a * 90));
	int16_t r = cfg.screenW / 40 + 2;
	for (uint8_t i = 0; i < 6; ++i) {
		float t = ((elapsed % 6000) + i * 600u) % 3600 / 3600.0f;
		int16_t x = int16_t(cfg.screenW * (i * 2 + 1) / 12 + sinf(t * 9 + i) * r);
		int16_t y = int16_t(cfg.screenH - t * cfg.screenH);
		x = eye_limit(x, r * 2, cfg.screenW - r * 2 - 1);
		y = eye_limit(y, r * 2, cfg.screenH - r * 2 - 1);
		gfx->fillCircle(x - r, y, r, ink);
		gfx->fillCircle(x + r, y, r, ink);
		gfx->fillTriangle(x - r * 2, y + 1, x + r * 2, y + 1, x, y + r * 2 + r / 2, ink);
	}
}

/** @brief adore scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	(void)layout;
	local_eye_extra_hearts(gfx, cfg, elapsed, a);
}

/** @brief Immutable adore face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "adore";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(255, 130, 175);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid adore face parameters");
} }

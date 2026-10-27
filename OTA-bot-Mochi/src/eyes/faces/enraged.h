/**
 * @file enraged.h
 * @brief enraged definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace enraged_face {
using namespace face_primitives;
inline void local_eye_extra_marks(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float a);


/** @brief enraged performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = ENRAGED; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * 0.5f; dx = a * 2.5f * sinf(phase * 0.045f); dy = a * 5;
}

/** @brief enraged local_eye_extra_marks implementation. */
inline void local_eye_extra_marks(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float a) {
	if (cfg.expression == LAUGHING) {
		uint16_t ink = eye_mix(eye_rgb(110, 195, 250), cfg.bgColor, uint8_t(a * 90));
		const EyeBox eyes[] = {layout.left, layout.right};
		for (const EyeBox &b : eyes) {
			float t = ((elapsed % 6000) + (b.left ? 0 : 250)) % 900 / 900.0f;
			int16_t x = b.left ? b.x : b.x + b.w;
			gfx->fillCircle(x, int16_t(b.y + b.h + t * cfg.screenH / 8), cfg.screenW / 80 + 1, ink);
		}
		return;
	}
	int16_t cx = cfg.screenW / 2, cy = cfg.screenH / 5, r = cfg.screenW / 14;
	uint16_t ink = eye_mix(eye_rgb(245, 60, 40), cfg.bgColor, uint8_t(a * 100));
	float spin = (elapsed % 6000) * 0.002f;
	for (uint8_t i = 0; i < 4; ++i) {
		float angle = 0.785398f + i * 1.570796f + 0.12f * sinf(spin * 6);
		int16_t x = int16_t(cx + cosf(angle) * r), y = int16_t(cy + sinf(angle) * r);
		gfx->drawLine(cx, cy, x, y, ink);
		gfx->drawLine(cx + 1, cy, x + 1, y, ink);
	}
}

/** @brief enraged scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	local_eye_extra_marks(gfx, cfg, layout, elapsed, a);
}

/** @brief Immutable enraged face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "enraged";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(245, 60, 40);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid enraged face parameters");
} }

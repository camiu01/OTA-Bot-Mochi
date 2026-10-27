/**
 * @file splash.h
 * @brief splash definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace splash_face {
using namespace face_primitives;
inline void local_eye_performance_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a);


/** @brief splash performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SPLASH; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * (0.3f + 0.15f * wave); dy = a * 6 * wave;
}

/** @brief splash local_eye_performance_water implementation. */
inline void local_eye_performance_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	uint16_t ink = eye_mix(eye_rgb(80, 180, 245), cfg.bgColor, uint8_t(a * 85));
	int16_t scale = eye_scale(cfg);
	for (uint8_t i = 0; i < 6; ++i) {
		float t = ((elapsed % 6000 + i * 137) % 1000) / 1000.0f;
		int16_t r = int16_t((2 + i % 2) * scale / 100);
		if (r < 1) { r = 1; }
		int16_t x = b.x + b.w * (i + 1) / 7;
		int16_t y = b.y + b.h + int16_t(t * cfg.screenH / 7);
		if (cfg.expression == SPRAYING) {
			x = int16_t((b.left ? b.x : b.x + b.w) + (b.left ? -1 : 1) * t * cfg.screenW / 8);
			y = int16_t(b.y + b.h / 2 + (int(i) - 3) * t * scale / 10);
		}
		if (cfg.expression == SPLASH) { x += int16_t(sinf(i * 1.2f + t * 6.2831853f) * b.w / 4); }
		x = eye_limit(x, r, cfg.screenW - r - 1);
		y = eye_limit(y, r, cfg.screenH - r - 1);
		gfx->fillCircle(x, y, r, ink);
		gfx->fillTriangle(x - r, y, x + r, y, x, y - r * 2, ink);
	}
	if (cfg.expression != SPLASH) { return; }
	for (int16_t x = b.x; x < b.x + b.w; ++x) {
		int16_t y = int16_t(b.y + b.h + sinf(x * 0.15f + (elapsed % 6000) * 0.0062831853f) * scale / 30);
		gfx->fillRect(x, y, 1, scale / 40 + 1, ink);
	}
}

/** @brief splash scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &b : eyes) { local_eye_performance_water(gfx, cfg, b, elapsed, a); }
}

/** @brief Immutable splash face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "splash";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(70, 185, 245);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid splash face parameters");
} }

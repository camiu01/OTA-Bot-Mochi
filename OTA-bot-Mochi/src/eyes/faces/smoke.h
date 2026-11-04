/**
 * @file smoke.h
 * @brief smoke definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace smoke_face {
using namespace face_primitives;
inline void local_eye_performance_air(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a);


/** @brief smoke performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SMOKE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sy -= a * 0.25f; dx = a * 3 * wave;
}

/** @brief smoke local_eye_performance_air implementation. */
inline void local_eye_performance_air(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	int16_t cx = b.x + b.w / 2, scale = eye_scale(cfg);
	uint16_t ink = eye_mix(eye_performance_ink(cfg, b.left, elapsed), cfg.bgColor, uint8_t(a * 45));
	for (uint8_t i = 0; i < 3; ++i) {
		uint32_t travel = elapsed + i * 566u;
		uint32_t event = travel / 1700, salt = (b.left ? 173u : 947u) + i * 71u;
		uint32_t start = uint32_t(face_random(event, salt + 11, cfg.variationSeed) * 220);
		uint32_t duration = 1100u + uint32_t(face_random(event, salt + 19, cfg.variationSeed) * 300);
		uint32_t phase = travel % 1700;
		if (phase < start || phase >= start + duration) { continue; }
		float t = (phase - start) / float(duration);
		float size = 0.8f + 0.4f * face_random(event, salt, cfg.variationSeed);
		float drift = face_random(event, salt + 37, cfg.variationSeed) * 2 - 1;
		int16_t r = int16_t((5 + 5 * t) * size * scale / 100);
		if (r < 1) { r = 1; }
		int16_t x = eye_limit(int16_t(cx + (sinf(t * 4 + i) + drift * t) * b.w / 4), r, cfg.screenW - r - 1);
		int16_t y = eye_limit(int16_t(b.y - t * cfg.screenH / 7), r, cfg.screenH - r - 1);
		uint16_t puff = eye_mix(ink, cfg.bgColor, uint8_t(sinf(t * 3.14159265f) * 100));
		gfx->fillCircle(x, y, r, puff);
	}
}

/** @brief smoke scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &b : eyes) { local_eye_performance_air(gfx, cfg, b, elapsed, a); }
}

/** @brief Immutable smoke face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "smoke";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(130, 195, 195);
	p.performanceMotion = performanceMotion;
	p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid smoke face parameters");
} }

/**
 * @file smoke.h
 * @brief smoke definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

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
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, scale = eye_scale(cfg);
	uint16_t ink = eye_mix(eye_performance_ink(cfg, b.left, elapsed), cfg.bgColor, uint8_t(a * 45));
	if (cfg.expression == SWINGING) {
		gfx->drawLine(cfg.screenW * (b.left ? 1 : 3) / 4,
			eye_limit(b.y - cfg.screenH / 6, 0, cfg.screenH - 1), cx, b.y, ink);
	} else if (cfg.expression == SMOKE || cfg.expression == SICK) {
		for (uint8_t i = 0; i < 3; ++i) {
			float t = ((elapsed % 6000 + i * 500) % 1500) / 1500.0f;
			int16_t r = int16_t((cfg.expression == SICK ? 3 : 5 + 5 * t) * scale / 100);
			if (r < 1) { r = 1; }
			int16_t x = eye_limit(int16_t(cx + sinf(t * 6.2831853f + i) * b.w / 3), r, cfg.screenW - r - 1);
			int16_t y = eye_limit(int16_t(b.y - t * cfg.screenH / 7), r, cfg.screenH - r - 1);
			uint16_t puff = eye_mix(ink, cfg.bgColor, uint8_t((1 - t) * 100));
			if (cfg.expression == SICK) { gfx->drawCircle(x, y, r, puff); }
			else { gfx->fillCircle(x, y, r, puff); }
		}
	} else if (cfg.expression == BLINDING || cfg.expression == BUZZING) {
		for (uint8_t i = 0; i < 6; ++i) {
			float angle = i * 1.04719755f + (elapsed % 6000) * 0.00104719755f;
			int16_t rx = b.w / 2 + scale / 10, ry = b.h / 2 + scale / 10;
			gfx->drawLine(int16_t(cx + cosf(angle) * rx), int16_t(cy + sinf(angle) * ry),
				int16_t(cx + cosf(angle) * (rx + scale / 12)),
				int16_t(cy + sinf(angle) * (ry + scale / 12)), ink);
		}
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

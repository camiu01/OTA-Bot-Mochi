/**
 * @file menacing.h
 * @brief menacing definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace menacing_face {
using namespace face_primitives;
inline void local_eye_performance_rotor(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a);


/** @brief menacing local_eye_performance_rotor implementation. */
inline void local_eye_performance_rotor(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
	int16_t r = (b.w < b.h ? b.w : b.h) / 2;
	EyeBox circle = {int16_t(cx - r), int16_t(cy - r), int16_t(r * 2), int16_t(r * 2), r, b.left};
	cfg.fgColor = eye_performance_ink(cfg, b.left, elapsed);
	eye_scene_body(gfx, cfg, eye_scene_box(b, circle, a), a);
	gfx->fillCircle(cx, cy, int16_t(r * 0.66f * a), cfg.bgColor);
	uint16_t ink = eye_mix(eye_rgb(255, 210, 165), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 12; ++i) {
		float angle = i * 0.523598776f +
			(elapsed % 6000) * (b.left ? 0.0020943951f : -0.0020943951f);
		int16_t radius = int16_t(r * 1.2f);
		int16_t x = int16_t(cx + cosf(angle) * radius), y = int16_t(cy + sinf(angle) * radius);
		gfx->fillCircle(x, y, int16_t((r / 12 + 1) * a), ink);
	}
}

/** @brief menacing performanceScene procedure. */
inline bool performanceScene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &b : eyes) { local_eye_performance_rotor(gfx, cfg, b, elapsed, a); }
	return true;
}

/** @brief Immutable menacing face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "menacing";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(240, 90, 110);
	p.performanceScene = performanceScene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid menacing face parameters");
} }

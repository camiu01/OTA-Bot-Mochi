/**
 * @file sushi.h
 * @brief sushi definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sushi_face {
using namespace face_primitives;
inline EyeBox local_eye_sushi_target(const EyeConfig &cfg);
inline void local_eye_sushi_topping(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &rice, float a);
inline void local_eye_performance_sushi(Arduino_GFX *gfx, EyeConfig cfg, const EyeLayout &layout,
		uint32_t elapsed, float a);


/** @brief sushi local_eye_sushi_target implementation. */
inline EyeBox local_eye_sushi_target(const EyeConfig &cfg) {
	int16_t w = cfg.screenW * 3 / 5;
	if (w > cfg.screenH) { w = cfg.screenH; }
	int16_t h = w / 2;
	return {int16_t((cfg.screenW - w) / 2), int16_t((cfg.screenH - h) / 2), w, h, int16_t(h / 3), true};
}

/** @brief sushi local_eye_sushi_topping implementation. */
inline void local_eye_sushi_topping(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &rice, float a) {
	uint16_t salmon = eye_mix(eye_rgb(245, 135, 110), cfg.fgColor, uint8_t(a * 100));
	int16_t h = int16_t(rice.h * 0.52f * a), pad = rice.w / 12;
	if (h < 2) { return; }
	gfx->fillRoundRect(rice.x + pad, rice.y, rice.w - 2 * pad, h, h / 3, salmon);
	uint16_t stripe = eye_mix(eye_rgb(255, 210, 175), salmon, uint8_t(a * 100));
	for (int16_t i = 1; i < 6; ++i) {
		int16_t x = rice.x + pad + (rice.w - 2 * pad) * i / 7;
		gfx->drawLine(x, rice.y + 2, x + rice.w / 12, rice.y + h - 2, stripe);
	}
}

/** @brief sushi local_eye_performance_sushi implementation. */
inline void local_eye_performance_sushi(Arduino_GFX *gfx, EyeConfig cfg, const EyeLayout &layout,
		uint32_t elapsed, float a) {
	EyeBox target = local_eye_sushi_target(cfg);
	EyeBox rice[] = {eye_scene_box(layout.left, target, a), eye_scene_box(layout.right, target, a)};
	cfg.fgColor = eye_performance_ink(cfg, true, elapsed);
	for (const EyeBox &b : rice) { eye_scene_body(gfx, cfg, b, a); }
	// At full merge paint the single topping once, not a pair of overlapping nigiri.
	local_eye_sushi_topping(gfx, cfg, rice[0], a);
	if (a < 1) { local_eye_sushi_topping(gfx, cfg, rice[1], a); }
}

/** @brief sushi performanceScene procedure. */
inline bool performanceScene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	local_eye_performance_sushi(gfx, cfg, layout, elapsed, a); return true;
}

/** @brief Immutable sushi face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sushi";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(245, 235, 215);
	p.performanceScene = performanceScene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sushi face parameters");
} }

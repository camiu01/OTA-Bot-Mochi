/**
 * @file wheels.h
 * @brief Two original five-spoke wheels rotate in opposite directions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_backgrounds.h"
namespace face_defs { namespace wheels_face {
/** @brief Morph style bodies into bounded tires with moving metal spokes. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	for (const EyeBox &b : {layout.left, layout.right}) {
		int16_t r = (b.w < b.h ? b.w : b.h) / 2;
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
		EyeBox target = {int16_t(cx - r), int16_t(cy - r), int16_t(2 * r), int16_t(2 * r), r, b.left};
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye_scene_body(gfx, ink, eye_scene_box(b, target, a), a);
		gfx->fillCircle(cx, cy, int16_t(r * 0.73f * a), cfg.bgColor);
		uint16_t metal = eye_mix(eye_rgb(210, 225, 240), cfg.bgColor, uint8_t(a * 100));
		for (uint8_t i = 0; i < 5; ++i) {
			float angle = i * 1.25663706f + (elapsed % 6000) * (b.left ? 0.00314159265f : -0.00314159265f);
			gfx->drawLine(cx, cy, int16_t(cx + cosf(angle) * r * 0.68f * a),
				int16_t(cy + sinf(angle) * r * 0.68f * a), metal);
		}
		gfx->fillCircle(cx, cy, int16_t(r * 0.17f * a), metal);
	}
	return true;
}
/** @brief Register the wheel performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "wheels"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(95, 130, 165); p.performanceScene = scene;
	p.background = face_backgrounds::road; p.bounds = face_backgrounds::full_bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid wheels face");
} }

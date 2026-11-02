/**
 * @file hollow_anger.h
 * @brief Hollow angry eyes lean inward and swell toward the viewer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace hollow_anger_face {
/** @brief Morph the configured pair into outlined, sloping eyes with a slow close-up. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	float zoom = 0.5f + 0.5f * sinf((elapsed % 6000) * 0.0010471976f - 1.5707963f);
	for (const EyeBox &source : {layout.left, layout.right}) {
		int16_t w = int16_t(cfg.screenW * (0.29f + 0.12f * zoom));
		int16_t h = int16_t(cfg.screenH * (0.15f + 0.08f * zoom));
		int16_t cx = int16_t(cfg.screenW * (source.left ? 0.26f : 0.74f));
		EyeBox target{int16_t(cx - w / 2), int16_t(cfg.screenH / 2 - h / 2), w, h, int16_t(h / 4), source.left};
		EyeBox b = eye_scene_box(source, target, a);
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye_scene_body(gfx, ink, b, a);
		face_primitives::slope(gfx, cfg, b, true, int16_t(b.h * 0.55f * a));
		int16_t pad = eye_limit(int16_t(b.w / 12), 2, int16_t(b.w / 3));
		int16_t cut = int16_t(b.h * 0.35f * a);
		if (cut > 0) {
			gfx->fillRoundRect(b.x + pad, b.y + b.h / 2, b.w - 2 * pad, cut, cut / 3, cfg.bgColor);
		}
	}
	uint16_t red = eye_mix(eye_rgb(245, 102, 86), cfg.bgColor, uint8_t(a * 85));
	face_primitives::sparkle(gfx, cfg.screenW * 9 / 10, cfg.screenH / 3, cfg.screenW / 20, red);
	return true;
}

/** @brief Register a reversible six-second close-up without changing the saved palette. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "hollow_anger"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(184, 220, 250); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid hollow anger face");
} }

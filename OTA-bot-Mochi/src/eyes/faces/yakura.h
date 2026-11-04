/**
 * @file yakura.h
 * @brief Round ember eyes beneath swept pale brows and drifting smoke ribbons.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace yakura_face {
/** @brief Sweep two tapered, gently curling ribbons outward from each brow. */
inline void brows(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, float a) {
	int16_t cx = b.x + b.w / 2, direction = b.left ? -1 : 1;
	float seed = face_random(elapsed / 6000, b.left ? 173u : 947u, cfg.variationSeed);
	float wave = (elapsed % 6000) * 0.0031415927f + seed * 6.2831853f;
	uint16_t ink = eye_mix(eye_rgb(187, 221, 247), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t strand = 0; strand < 2; ++strand) {
		int16_t px = cx - direction * b.w / 3;
		int16_t py = b.y + b.h / 10;
		for (uint8_t step = 1; step <= 20; ++step) {
			float t = step / 20.0f;
			int16_t x = int16_t(cx + direction * b.w * (t * 1.15f - 0.33f));
			int16_t y = int16_t(b.y - a * b.h * (t * (0.65f + strand * 0.22f)
				+ sinf(t * 7 + wave + strand) * t * 0.15f));
			x = eye_limit(x, 0, int16_t(cfg.screenW - 1));
			y = eye_limit(y, 0, int16_t(cfg.screenH - 1));
			int16_t thick = int16_t(a * b.h * 0.12f * (1 - t)) + 1;
			for (int16_t row = 0; row < thick; ++row) { gfx->drawLine(px, py - row, x, y - row, ink); }
			px = x; py = y;
		}
	}
}

/** @brief Morph the selected style into glowing discs, keeping the face mouth-free. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	int16_t diameter = cfg.screenW / 5;
	if (diameter > cfg.screenH / 6) { diameter = cfg.screenH / 6; }
	for (const EyeBox &source : {layout.left, layout.right}) {
		float sway = sinf((elapsed % 6000) * 0.0041887902f + (source.left ? 0 : 0.6f));
		int16_t cx = int16_t(cfg.screenW * (source.left ? 0.32f : 0.68f));
		int16_t cy = int16_t(cfg.screenH * 0.53f + sway * cfg.screenH / 70);
		EyeBox target{int16_t(cx - diameter / 2), int16_t(cy - diameter / 2),
			diameter, diameter, int16_t(diameter / 2), source.left};
		EyeBox b = eye_scene_box(source, target, a);
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, source.left, elapsed);
		eye_scene_body(gfx, ink, b, a);
		uint16_t glow = eye_mix(eye_rgb(255, 191, 117), ink.fgColor, uint8_t(a * 100));
		int16_t s = int16_t(diameter * a / 9);
		if (s > 0) { gfx->fillCircle(b.x + b.w / 3, b.y + b.h / 3, s, glow); }
		brows(gfx, cfg, b, elapsed, a);
	}
	return true;
}

/** @brief Register a six-second pale-smoke performance distinct from Yokai's horns. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "yakura"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(242, 120, 68); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid yakura face");
} }

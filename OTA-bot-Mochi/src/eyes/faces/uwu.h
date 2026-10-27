/**
 * @file uwu.h
 * @brief Tall U-shaped eyes and tilted cheek strokes, without a mouth.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace uwu_face {
/** @brief One brief closed-eye squeeze interrupts the tall U pose, then recovers. */
inline EyeBox target(const EyeConfig &cfg, bool left, uint32_t elapsed) {
	float pulse = 1 - fabsf(float(elapsed % 6000) - 2850) / 300;
	pulse = pulse < 0 ? 0 : pulse * pulse * (3 - 2 * pulse);
	int16_t w = cfg.screenW * 19 / 100;
	int16_t tall = eye_limit(cfg.screenH * 29 / 100, w * 6 / 5, cfg.screenH * 2 / 5);
	int16_t h = int16_t(tall * (1 - pulse * 0.65f));
	int16_t x = cfg.screenW / 2 + (left ? -1 : 1) * cfg.screenW / 5 - w / 2;
	int16_t y = cfg.screenH * 43 / 100 - h / 2;
	int16_t r = (w < h ? w : h) / 2;
	return {x, y, w, h, r, left};
}
/** @brief Draw one short rounded cheek leaning upward toward the outside. */
inline void cheek(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &cup, uint16_t ink) {
	int16_t cx = cup.x + cup.w / 2;
	int16_t cy = cup.y + cup.h + cfg.screenH / 28;
	int16_t radius = cfg.screenH / 100 + 1;
	for (int16_t x = -cup.w / 4; x <= cup.w / 4; ++x) {
		gfx->fillCircle(cx + x, cy + (cup.left ? 1 : -1) * x / 4, radius, ink);
	}
}
/** @brief Carve top-open tall cups and fade in cheeks below them, never a mouth. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	for (const EyeBox &b : {layout.left, layout.right}) {
		EyeBox cup = eye_scene_box(b, target(cfg, b.left, elapsed), a);
		cup.radius = eye_limit(cup.radius, 0, (cup.w < cup.h ? cup.w : cup.h) / 2);
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye_scene_body(gfx, ink, cup, a);
		uint16_t blush = eye_mix(ink.fgColor, cfg.bgColor, uint8_t(a * 100));
		cheek(gfx, cfg, cup, blush);
		int16_t edge = cfg.screenW / 40 + 1;
		int16_t w = int16_t((cup.w - edge * 2) * a);
		int16_t h = int16_t((cup.h - edge) * a);
		if (w <= 0 || h <= 0) { continue; }
		int16_t radius = (w < h + edge ? w : h + edge) / 2;
		gfx->fillRoundRect(cup.x + (cup.w - w) / 2, cup.y - edge, w, h + edge, radius, cfg.bgColor);
	}
	return true;
}
/** @brief Register the tall-eyed UwU performance with cheeks but no mouth. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "uwu"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(190, 220, 255); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid uwu face");
} }

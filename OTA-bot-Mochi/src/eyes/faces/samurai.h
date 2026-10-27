/**
 * @file samurai.h
 * @brief Asymmetric slanted eyes above a broad diagonal blade and traveling glint.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace samurai_face {
/** @brief Morph toward a smaller low left eye and a larger high right eye. */
inline void layout(const EyeConfig &cfg, EyeBox &b, uint32_t, float a) {
	EyeBox target = b;
	target.w = eye_limit(int16_t(b.w * (b.left ? 0.65f : 1.12f)), 6, cfg.screenW / 3);
	target.h = eye_limit(int16_t(b.h * (b.left ? 0.42f : 0.72f)), 6, cfg.screenH / 3);
	target.x = cfg.screenW / 2 + (b.left ? -1 : 1) * cfg.screenW / 5 - target.w / 2;
	target.y = cfg.screenH / 2 - target.h / 2 + (b.left ? 1 : -1) * cfg.screenH / 20;
	target.radius = eye_limit(b.radius / 2, 1, (target.w < target.h ? target.w : target.h) / 2);
	b = eye_scene_box(b, target, a);
	b.radius = eye_limit(b.radius, 0, (b.w < b.h ? b.w : b.h) / 2);
}
/** @brief Upper blade edge rises across the lower face, without a full-screen flash. */
inline int16_t blade_top(const EyeConfig &cfg, int16_t x) {
	return cfg.screenH * 7 / 10 - int32_t(x) * cfg.screenH / (cfg.screenW * 6);
}
/** @brief Paint a steel ribbon in graded parallel strips, then sweep a narrow glint. */
inline void blade(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	int16_t edge = cfg.screenW / 20, right = cfg.screenW - edge;
	int16_t thick = cfg.screenH / 22 + 2;
	for (uint8_t i = 0; i < 6; ++i) {
		int16_t top = i * thick / 6, bottom = (i + 1) * thick / 6;
		uint16_t steel = eye_mix(eye_rgb(85 + i * 20, 130 + i * 17, 170 + i * 14),
			cfg.bgColor, uint8_t(a * 100));
		int16_t y1 = blade_top(cfg, edge), y2 = blade_top(cfg, right);
		gfx->fillTriangle(edge, y1 + top, right, y2 + top, right, y2 + bottom, steel);
		gfx->fillTriangle(edge, y1 + top, right, y2 + bottom, edge, y1 + bottom, steel);
	}
	int16_t x = edge + int32_t(elapsed % 3000) * (right - edge) / 3000;
	uint16_t glint = eye_mix(eye_rgb(220, 240, 255), cfg.bgColor, uint8_t(a * 100));
	gfx->drawLine(x, blade_top(cfg, x) + 1, x + cfg.screenW / 60,
		blade_top(cfg, x) + thick - 1, glint);
}
/** @brief Paint one complete inclined eye and its upward wisps. */
inline void eye(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, float a) {
	EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
	eye_scene_body(gfx, ink, b, a);
	int16_t depth = int16_t(b.h * 0.55f * a);
	gfx->fillTriangle(b.x, b.y, b.x + b.w - 1, b.y, b.x, b.y + depth, cfg.bgColor);
	uint16_t wisp = eye_mix(eye_rgb(115, 170, 205), cfg.bgColor, uint8_t(a * 70));
	int16_t cx = b.x + b.w / 2, top = b.y - cfg.screenH / 45;
	for (int16_t i = -1; i <= 1; ++i) {
		int16_t sway = int16_t(sinf((elapsed % 6000) * 0.0020943951f + i) * 2);
		gfx->drawLine(cx + i * 3, top, cx + i * 4 - cfg.screenW / 50 + sway,
			top - cfg.screenH / 18, wisp);
	}
}
/** @brief Compose the inclined eye pair and diagonal blade. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	// Fading ink is blended against the background, so it must not overwrite either eye.
	blade(gfx, cfg, elapsed, a);
	for (const EyeBox &b : {layout.left, layout.right}) { eye(gfx, cfg, b, elapsed, a); }
	return true;
}
/** @brief Register the original samurai-themed performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "samurai"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(170, 215, 250); p.layout = layout; p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid samurai face");
} }

/**
 * @file uwu.h
 * @brief White top-open U eyes with pink cheeks, without a mouth or scenery.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace uwu_face {
/** @brief Keep the tall U silhouettes centered and separated on every panel. */
inline EyeBox target(const EyeConfig &cfg, bool left, uint32_t) {
	int16_t w = int16_t(cfg.screenW * 19 / 100);
	int16_t h = eye_limit(int16_t(cfg.screenH * 29 / 100), w * 6 / 5, cfg.screenH * 2 / 5);
	int16_t cx = int16_t(cfg.screenW / 2 + (left ? -1 : 1) * cfg.screenW / 5);
	int16_t y = int16_t(cfg.screenH * 43 / 100 - h / 2);
	return {int16_t(cx - w / 2), y, w, h, int16_t(w / 2), left};
}
/** @brief Small rounded blush marks use their own pink, never the white eye ink. */
inline void cheek(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &cup, float a) {
	int16_t w = int16_t(cup.w * 2 / 3);
	int16_t h = int16_t(cfg.screenH / 35 + 1);
	int16_t x = int16_t(cup.x + (cup.w - w) / 2);
	int16_t y = int16_t(cup.y + cup.h + cfg.screenH / 25);
	uint16_t pink = eye_mix(eye_rgb(255, 85, 139), cfg.bgColor, uint8_t(a * 100));
	gfx->fillRoundRect(x, y, w, h, h / 2, pink);
}
/** @brief Morph into white U cups, then paint separate pink cheeks without a mouth. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	for (const EyeBox &b : {layout.left, layout.right}) {
		EyeBox cup = eye_scene_box(b, target(cfg, b.left, elapsed), a);
		cup.radius = eye_limit(cup.radius, 0, (cup.w < cup.h ? cup.w : cup.h) / 2);
		EyeConfig ink = cfg;
		ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye_scene_body(gfx, ink, cup, a);
		int16_t edge = int16_t(cfg.screenW / 40 + 1);
		int16_t w = int16_t((cup.w - edge * 2) * a);
		int16_t h = int16_t((cup.h - edge) * a);
		if (w > 0 && h > 0) {
			int16_t r = (w < h + edge ? w : h + edge) / 2;
			gfx->fillRoundRect(cup.x + (cup.w - w) / 2, cup.y - edge, w, h + edge, r, cfg.bgColor);
		}
		cheek(gfx, cfg, cup, a);
	}
	return true;
}
/** @brief Register white U eyes and pink cheeks; deliberately no backdrop. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "uwu"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(255, 255, 255); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid uwu face");
} }

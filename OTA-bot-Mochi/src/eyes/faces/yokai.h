/**
 * @file yokai.h
 * @brief Original ember eyes with floating wisps and a paired horn silhouette.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace yokai_face {
/** @brief Draw small horns and wandering wisps rather than character artwork. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	uint16_t ink = eye_mix(eye_rgb(220, 135, 100), cfg.bgColor, uint8_t(a * 100));
	for (const EyeBox &b : {layout.left, layout.right}) {
		int16_t cx = b.x + b.w / 2, top = b.y - cfg.screenH / 35;
		int16_t s = cfg.screenW / 18;
		gfx->fillTriangle(cx - s, top, cx + s, top, cx + (b.left ? -s : s), top - s, ink);
	}
	for (uint8_t i = 0; i < 6; ++i) {
		float angle = (elapsed % 6000) * 0.00104719755f + i * 1.04719755f;
		int16_t x = int16_t(cfg.screenW / 2 + cosf(angle) * cfg.screenW * 0.42f);
		int16_t y = int16_t(cfg.screenH / 2 + sinf(angle) * cfg.screenH * 0.32f);
		face_primitives::flame(gfx, x, y, cfg.screenW / 50 + 1, ink);
	}
}
/** @brief Give the ember eyes an opposing, slow sway. */
inline void motion(bool left, uint32_t phase, float a, float &, float &sy, float &, float &dy) {
	sy -= a * 0.25f; dy = a * (left ? 4 : -4) * sinf(phase * 0.0020943951f);
}
/** @brief Register the original yokai performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "yokai"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(245, 130, 75); p.scenery = scenery; p.performanceMotion = motion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid yokai face");
} }

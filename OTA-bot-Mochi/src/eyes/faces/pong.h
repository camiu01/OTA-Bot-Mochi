/**
 * @file pong.h
 * @brief pong definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace pong_face {
using namespace face_primitives;
inline void local_eye_scene_pong(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t);


/** @brief pong local_eye_scene_pong implementation. */
inline void local_eye_scene_pong(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	static PongGame demo;
	if (elapsed % 12000 < 2000) { demo.reset(elapsed); }
	else if (t == 1) { demo.tick(elapsed, true); }
	int16_t x = layout.left.x, y = layout.left.y;
	int16_t w = layout.right.x + layout.right.w - x;
	int16_t h = layout.left.h > layout.right.h ? layout.left.h : layout.right.h;
	uint8_t opacity = uint8_t(t * t * 100);
	mini_pong_draw(gfx, demo, x, y, w, h, eye_mix(cfg.fgColor, cfg.bgColor, opacity));
	EyeBox targets[] = {
		{int16_t(x + w * 8 / 240), int16_t(y + int16_t(demo.left - 16) * h / 160),
			int16_t(w * 4 / 240 + 1), int16_t(h * 32 / 160), 0, true},
		{int16_t(x + w * 228 / 240), int16_t(y + int16_t(demo.right - 16) * h / 160),
			int16_t(w * 4 / 240 + 1), int16_t(h * 32 / 160), 0, false}
	};
	eye_scene_body(gfx, cfg, eye_scene_box(layout.left, targets[0], t), t);
	eye_scene_body(gfx, cfg, eye_scene_box(layout.right, targets[1], t), t);
}

/** @brief pong scene procedure. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	local_eye_scene_pong(gfx, cfg, layout, elapsed, a); return true;
}

/** @brief Immutable pong face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "pong";
	p.fullScene = true;
	p.scene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid pong face parameters");
} }

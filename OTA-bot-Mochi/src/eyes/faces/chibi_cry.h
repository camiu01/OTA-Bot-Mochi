/**
 * @file chibi_cry.h
 * @brief Squeezed, trembling eyes spray two comic fountain arcs of tears.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace chibi_cry_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Six droplets per side travel along parabolic arcs and repeat. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, 0, int16_t(sinf(ms * 0.03f) * a * 1.5f), 1 + a * 0.05f, 1 - a * 0.85f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t water = tint(cfg, eye_rgb(90, 170, 255), a);
	for (auto b : {pair.left, pair.right}) {
		int16_t sx = b.left ? b.x : b.x + b.w, sy = b.y + b.h / 2, dir = b.left ? -1 : 1;
		for (int i = 0; i < 6; ++i) {
			float p = ((ms / 6 + i * 60) % 360) / 360.0f;
			int16_t x = sx + dir * int16_t(p * W / 5);
			int16_t y = sy - int16_t(p * (1 - p) * 4 * H / 10) + int16_t(p * H / 6);
			gfx->fillCircle(x, y, W / 90 + 1, water);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("chibi_cry", scene);
static_assert(face_preset_valid(preset), "Invalid Chibi Cry face");
} }

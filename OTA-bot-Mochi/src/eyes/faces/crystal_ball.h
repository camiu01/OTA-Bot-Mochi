/**
 * @file crystal_ball.h
 * @brief Violet-lit eyes gaze down into a crystal ball where pale mist swirls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace crystal_ball_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Six mist motes orbit at breathing radii inside the orb. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float swirl = ms * 0.004f;
	if (a > 0) {
		uint16_t orb = tint(cfg, eye_rgb(120, 70, 190), a), mist = tint(cfg, eye_rgb(210, 170, 255), a);
		uint16_t stand = tint(cfg, eye_rgb(160, 110, 60), a);
		int16_t r = W / 9 + 3, cx = W / 2, cy = H - r - H / 12;
		gfx->fillRect(cx - r, cy + r - 2, r * 2, H / 20 + 2, stand);
		gfx->fillCircle(cx, cy, r, orb);
		for (int i = 0; i < 6; ++i) {
			float ang = swirl + i * 1.047f, d = r * 0.55f * (0.6f + 0.4f * sinf(swirl * 2 + i));
			gfx->fillCircle(cx + int16_t(cosf(ang) * d), cy + int16_t(sinf(ang) * d), 2, mist);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * H / 40), 1, 1 - a * 0.3f, eye_rgb(200, 160, 255));
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("crystal_ball", scene);
static_assert(face_preset_valid(preset), "Invalid Crystal Ball face");
} }

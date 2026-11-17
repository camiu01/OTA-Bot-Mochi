/**
 * @file fish_bowl.h
 * @brief Slightly shrunken eyes inside a round glass bowl as a goldfish swims past.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace fish_bowl_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The fish turns at each end and trails rising bubbles. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	if (a > 0) {
		uint16_t glass = tint(cfg, eye_rgb(140, 200, 235), a), fish = tint(cfg, eye_rgb(255, 140, 40), a);
		uint16_t bubble = tint(cfg, eye_rgb(190, 230, 255), a);
		int16_t r = (W < H ? W : H) * 9 / 20, cx = W / 2, cy = H / 2 + H / 20;
		gfx->drawCircle(cx, cy, r, glass);
		gfx->drawCircle(cx, cy, r - 1, glass);
		gfx->drawFastHLine(cx - r * 3 / 4, cy - r * 2 / 3, r * 3 / 2, glass);
		int16_t s = W / 30 + 2, dir = cosf(ms * 0.0025f) > 0 ? 1 : -1;
		int16_t fx = cx + int16_t(sinf(ms * 0.0025f) * r * 0.6f), fy = cy + r / 2;
		gfx->fillEllipse(fx, fy, s * 2, s, fish);
		gfx->fillTriangle(fx - dir * s * 2, fy, fx - dir * s * 3, fy - s, fx - dir * s * 3, fy + s, fish);
		for (int i = 0; i < 3; ++i) {
			gfx->drawCircle(fx + dir * s * 2, fy - 4 - int16_t((ms / 20 + i * 15) % (r / 2 + 1)), 2, bubble);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 - a * 0.1f, 1 - a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("fish_bowl", scene);
static_assert(face_preset_valid(preset), "Invalid Fish Bowl face");
} }

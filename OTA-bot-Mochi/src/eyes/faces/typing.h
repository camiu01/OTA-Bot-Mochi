/**
 * @file typing.h
 * @brief Lowered, focused eyes watch pseudo-random keys light up on a small keyboard.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace typing_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief A new key lights every 110 ms from a fixed integer hash. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * H / 30), 1, 1 - a * 0.4f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t key = tint(cfg, eye_rgb(90, 95, 115), a), lit = tint(cfg, eye_rgb(130, 230, 255), a);
	int16_t kw = W / 12, kh = H / 22 + 2, x0 = W / 2 - kw * 3, y0 = H - kh * 3 - 6;
	uint32_t pressed = (((ms / 110) * 2654435761u) >> 7) % 18;
	for (int r = 0; r < 3; ++r) {
		for (int c = 0; c < 6; ++c) {
			bool on = pressed == uint32_t(r * 6 + c);
			gfx->fillRoundRect(x0 + c * kw + 1, y0 + r * (kh + 1), kw - 2, kh, 2, on ? lit : key);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("typing", scene);
static_assert(face_preset_valid(preset), "Invalid Typing face");
} }

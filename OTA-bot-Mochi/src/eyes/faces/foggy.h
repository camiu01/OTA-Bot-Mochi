/**
 * @file foggy.h
 * @brief Squinting eyes peer side to side through slowly drifting fog bands.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace foggy_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Three striped bands slide at different speeds over and around the pair. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t peek = int16_t(sinf(ms * 0.0025f) * a * W / 40);
	pair_body(gfx, cfg, pair, a, peek, 0, 1, 1 - a * 0.5f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t fog = tint(cfg, eye_rgb(170, 175, 185), a);
	for (int k = 0; k < 3; ++k) {
		int16_t y = pair.left.y - H / 20 + k * (pair.left.h / 2 + H / 30);
		int16_t shift = int16_t((ms / (30 + k * 7) + k * W / 3) % W);
		for (int n = 0; n < 4; ++n) {
			for (int j = 0; j < 3; ++j) { gfx->drawFastHLine(shift - W + n * W / 2, y + j * 3, W / 3, fog); }
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("foggy", scene);
static_assert(face_preset_valid(preset), "Invalid Foggy face");
} }

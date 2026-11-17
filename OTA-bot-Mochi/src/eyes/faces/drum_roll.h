/**
 * @file drum_roll.h
 * @brief Jittering, tense eyes above a red snare drum struck by alternating sticks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace drum_roll_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Sticks swap every 90 ms; the pair jolts on each stroke. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	bool beat = (ms / 90) & 1;
	if (a > 0) {
		uint16_t shell = tint(cfg, eye_rgb(210, 50, 60), a), head = tint(cfg, eye_rgb(235, 230, 220), a);
		uint16_t stick = tint(cfg, eye_rgb(205, 160, 100), a);
		int16_t cx = W / 2, top = H - H / 5, rx = W / 6, ry = H / 30 + 2;
		gfx->fillRect(cx - rx, top, rx * 2, H / 8, shell);
		gfx->fillEllipse(cx, top, rx, ry, head);
		for (int s = -1; s <= 1; s += 2) {
			bool down = (s < 0) == beat;
			int16_t tx = cx + s * rx / 3, ty = down ? top : top - H / 12;
			gfx->drawLine(tx, ty, tx + s * W / 8, ty - H / 10, stick);
			gfx->drawLine(tx + 1, ty, tx + 1 + s * W / 8, ty - H / 10, stick);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, beat ? int16_t(a * 2) : 0, 1, 1 - a * 0.5f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("drum_roll", scene);
static_assert(face_preset_valid(preset), "Invalid Drum Roll face");
} }

/**
 * @file bubble_tea.h
 * @brief Eyes squeeze with each sip from a milk-tea cup of bouncing tapioca pearls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace bubble_tea_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Four pearls bounce at the bottom while the pair narrows rhythmically. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float sip = 0.5f + 0.5f * sinf(ms * 0.005f);
	if (a > 0) {
		uint16_t cup = tint(cfg, eye_rgb(225, 200, 170), a), pearl = tint(cfg, eye_rgb(90, 55, 40), a);
		uint16_t straw = tint(cfg, eye_rgb(120, 200, 230), a);
		int16_t w = W / 6, h = H / 5, x = W / 2 - w / 2, y = H - h - 4;
		gfx->fillRoundRect(x, y, w, h, 4, cup);
		gfx->drawLine(x + w / 2, y - H / 12, x + w * 2 / 3, y + h - 4, straw);
		for (int i = 0; i < 4; ++i) {
			int16_t px = x + w * (i + 1) / 5, py = y + h - 5 - int16_t(fabsf(sinf(ms * 0.006f + i)) * h / 3);
			gfx->fillCircle(px, py, W / 80 + 1, pearl);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 - a * 0.05f * sip, 1 - a * (0.3f + 0.4f * sip), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("bubble_tea", scene);
static_assert(face_preset_valid(preset), "Invalid Bubble Tea face");
} }

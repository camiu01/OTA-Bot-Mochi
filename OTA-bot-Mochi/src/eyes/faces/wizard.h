/**
 * @file wizard.h
 * @brief Focused eyes under a starry pointed hat, beside a wand trailing sparkles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace wizard_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The hat tip and wand sway; four sparkles circle the wand tip. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float wave = sinf(ms * 0.006f);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	if (a > 0) {
		uint16_t hat = tint(cfg, eye_rgb(60, 70, 180), a), star = tint(cfg, eye_rgb(255, 225, 90), a);
		int16_t brim = top - H / 30, w = W * 2 / 5;
		gfx->fillTriangle(W / 2 - w / 2, brim, W / 2 + w / 2, brim, W / 2 + int16_t(wave * W / 20), brim - H / 4, hat);
		gfx->fillRoundRect(W / 2 - w * 2 / 3, brim - 2, w * 4 / 3, H / 40 + 3, 2, hat);
		gfx->fillCircle(W / 2 - w / 8, brim - H / 12, 2, star);
		gfx->fillCircle(W / 2 + w / 10, brim - H / 7, 2, star);
		int16_t wx = W - W / 6, wy = H - H / 6 + int16_t(wave * H / 20), tx = wx - W / 8, ty = wy - H / 10;
		gfx->drawLine(wx, wy, tx, ty, tint(cfg, eye_rgb(140, 90, 50), a));
		for (int i = 0; i < 4; ++i) {
			int16_t sx = tx + int16_t(cosf(ms * 0.01f + i * 1.57f) * W / 20), sy = ty + int16_t(sinf(ms * 0.01f + i * 1.57f) * W / 20);
			gfx->drawLine(sx - 2, sy, sx + 2, sy, star);
			gfx->drawLine(sx, sy - 2, sx, sy + 2, star);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1, 1 - a * 0.4f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("wizard", scene);
static_assert(face_preset_valid(preset), "Invalid Wizard face");
} }

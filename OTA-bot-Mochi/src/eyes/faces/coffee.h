/**
 * @file coffee.h
 * @brief Drowsy eyes snap wide awake once the steaming coffee mug kicks in.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace coffee_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Eyes stay nearly shut for 2.6 seconds, then open past their rest size. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 6000;
	float wake = t < 2600 ? 0 : (t - 2600) / 500.0f;
	wake = wake > 1 ? 1 : wake;
	if (a > 0) {
		uint16_t mug = tint(cfg, eye_rgb(150, 95, 60), a), steam = tint(cfg, eye_rgb(205, 205, 215), a);
		int16_t w = W / 6, h = H / 7, x = W / 2 - w / 2, y = H - h - H / 14;
		gfx->fillRoundRect(x, y, w, h, 3, mug);
		gfx->drawCircle(x + w + h / 4, y + h / 2, h / 4, mug);
		for (int i = 0; i < 3; ++i) {
			int16_t sx = x + w * (i + 1) / 4, sy = y - 3 - int16_t((ms / 25 + i * 17) % (H / 9));
			gfx->drawLine(sx, sy, sx + int16_t(sinf(ms * 0.012f + i) * 3), sy - H / 30, steam);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 + a * wake * 0.05f, 1 - a * (0.8f - wake * 0.95f), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("coffee", scene);
static_assert(face_preset_valid(preset), "Invalid Coffee face");
} }

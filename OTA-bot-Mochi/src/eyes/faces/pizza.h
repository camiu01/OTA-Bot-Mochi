/**
 * @file pizza.h
 * @brief Hopping eyes over a pepperoni slice whose cheese string stretches upward.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace pizza_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The tip lifts and the cheese string stretches with a slow pull cycle. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float pull = 0.5f + 0.5f * sinf(ms * 0.0035f);
	int16_t hop = int16_t(fabsf(sinf(ms * 0.008f)) * a * H / 40);
	if (a > 0) {
		uint16_t crust = tint(cfg, eye_rgb(225, 165, 80), a), cheese = tint(cfg, eye_rgb(255, 215, 90), a);
		uint16_t pep = tint(cfg, eye_rgb(205, 50, 45), a);
		int16_t s = W / 9 + 2, cx = W / 2, y = H - H / 10;
		int16_t tip = y - s * 2 - int16_t(pull * H / 14);
		gfx->fillTriangle(cx - s, y, cx + s, y, cx, tip, cheese);
		gfx->fillRect(cx - s, y, s * 2, 3, crust);
		gfx->fillCircle(cx - s / 3, y - s / 2, s / 5 + 1, pep);
		gfx->fillCircle(cx + s / 3, y - s / 3, s / 5 + 1, pep);
		gfx->drawLine(cx, tip, cx + 2, tip - int16_t(pull * H / 10), cheese);
	}
	pair_body(gfx, cfg, pair, a, 0, -hop, 1, 1 - a * 0.2f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("pizza", scene);
static_assert(face_preset_valid(preset), "Invalid Pizza face");
} }

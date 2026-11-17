/**
 * @file faint.h
 * @brief Eyes sag and close unevenly while a little ghostly soul floats up and away.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace faint_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The collapse takes 1.5 seconds; the soul then rises and sways. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 6000;
	float drop = t < 1500 ? t / 1500.0f : 1;
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(a * drop * (b.left ? H / 14 : H / 9));
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * drop * 0.85f, cfg.fgColor);
	}
	if (a <= 0 || drop < 1) { return true; }
	uint16_t soul = tint(cfg, eye_rgb(200, 225, 255), a);
	float rise = (t - 1500) / 4500.0f;
	int16_t x = W / 2 + int16_t(sinf(ms * 0.006f) * W / 20), y = pair.left.y - int16_t(rise * H / 3);
	int16_t r = W / 20 + 2;
	gfx->fillCircle(x, y, r, soul);
	gfx->fillTriangle(x - r, y, x + r, y, x + int16_t(sinf(ms * 0.01f) * r), y + r * 2 + 2, soul);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("faint", scene);
static_assert(face_preset_valid(preset), "Invalid Faint face");
} }

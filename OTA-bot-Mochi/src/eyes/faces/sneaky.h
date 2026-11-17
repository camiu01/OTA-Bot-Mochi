/**
 * @file sneaky.h
 * @brief Low, narrow eyes creep from side to side while tiptoe footprints appear.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace sneaky_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The pair slides over three seconds; footprints build up in order. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 3000;
	float slide = t < 1500 ? t / 1500.0f : 1 - (t - 1500) / 1500.0f;
	pair_body(gfx, cfg, pair, a, int16_t((slide - 0.5f) * 2 * a * W / 14), int16_t(a * H / 25), 1 + a * 0.1f, 1 - a * 0.65f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t step = tint(cfg, eye_rgb(140, 140, 160), a);
	uint8_t shown = uint8_t(ms / 450 % 5);
	for (uint8_t i = 0; i < shown; ++i) {
		gfx->fillEllipse(W / 6 + i * W / 6, H - H / 10 - (i & 1) * H / 25, W / 30 + 3, H / 40 + 3, step);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("sneaky", scene);
static_assert(face_preset_valid(preset), "Invalid Sneaky face");
} }

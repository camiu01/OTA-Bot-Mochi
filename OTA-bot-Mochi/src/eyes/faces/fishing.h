/**
 * @file fishing.h
 * @brief Patient eyes stare at a floating bobber, then widen when it dips on a bite.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace fishing_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief A bite lasts 800 ms of every four seconds; the water ripples throughout. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 4000;
	bool bite = t > 2600 && t < 3400;
	int16_t water = H - H / 6, bx = W * 3 / 5, by = water - 3 + int16_t(sinf(ms * 0.004f) * 2 + (bite ? 6 : 0));
	if (a > 0) {
		uint16_t blue = tint(cfg, eye_rgb(70, 140, 220), a), cord = tint(cfg, eye_rgb(200, 200, 210), a);
		uint16_t red = tint(cfg, eye_rgb(240, 60, 60), a), white = tint(cfg, eye_rgb(250, 250, 250), a);
		for (int16_t x = 0; x < W; x += 6) {
			gfx->drawLine(x, water + int16_t(sinf(x * 0.2f + ms * 0.004f) * 2), x + 6, water + int16_t(sinf((x + 6) * 0.2f + ms * 0.004f) * 2), blue);
		}
		int16_t r = W / 36 + 3;
		gfx->drawLine(W - 2, 2, bx, by - 4, cord);
		gfx->fillCircle(bx, by, r, red);
		gfx->fillRect(bx - r, by, r * 2 + 1, r + 1, white);
	}
	pair_body(gfx, cfg, pair, a, int16_t(a * W / 50), int16_t(a * H / 50), 1 + a * (bite ? 0.1f : 0), 1 - a * (bite ? -0.1f : 0.4f), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("fishing", scene);
static_assert(face_preset_valid(preset), "Invalid Fishing face");
} }

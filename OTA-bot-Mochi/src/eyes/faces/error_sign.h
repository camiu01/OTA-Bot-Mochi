/**
 * @file error_sign.h
 * @brief Shaky, wide eyes under a red warning triangle that pulses brighter and dimmer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace error_sign_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The warning holds bright for 600 ms of every 900 ms. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	bool on = (ms % 900) < 600;
	pair_body(gfx, cfg, pair, a, int16_t(sinf(ms * 0.05f) * a * 2), 0, 1 - a * 0.15f, 1 + a * 0.12f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t red = tint(cfg, on ? eye_rgb(255, 60, 60) : eye_rgb(255, 140, 140), a);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t s = W / 12 + 3, cx = W / 2, by = top - H / 25;
	for (int k = 0; k < 2; ++k) {
		gfx->drawLine(cx - s + k, by - k, cx + s - k, by - k, red);
		gfx->drawLine(cx - s + k, by - k, cx, by - s * 2 + k, red);
		gfx->drawLine(cx + s - k, by - k, cx, by - s * 2 + k, red);
	}
	gfx->drawFastVLine(cx, by - s * 3 / 2, s - 2, red);
	gfx->fillCircle(cx, by - s / 3, 1, red);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("error_sign", scene);
static_assert(face_preset_valid(preset), "Invalid Error Sign face");
} }

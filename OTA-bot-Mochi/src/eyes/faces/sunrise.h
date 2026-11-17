/**
 * @file sunrise.h
 * @brief Closed eyes slowly open as an orange sun rises over a pink horizon.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace sunrise_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The opening follows the sun height; rays appear once it is half risen. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float rise = (ms % 6000) / 4500.0f;
	rise = rise > 1 ? 1 : rise;
	if (a > 0) {
		uint16_t sun = tint(cfg, eye_rgb(255, 150, 60), a), sky = tint(cfg, eye_rgb(255, 110, 140), a);
		int16_t horizon = H * 6 / 7, r = W / 7 + 2;
		int16_t cy = horizon + r - int16_t(rise * r * 1.6f);
		gfx->fillCircle(W / 2, cy, r, sun);
		gfx->fillRect(0, horizon, W, H - horizon, cfg.bgColor);
		gfx->drawFastHLine(0, horizon, W, sky);
		for (int i = 1; rise > 0.5f && i < 6; ++i) {
			float t = 3.14159f * i / 6;
			gfx->drawLine(W / 2 + int16_t(cosf(t) * r * 1.3f), cy - int16_t(sinf(t) * r * 1.3f),
				W / 2 + int16_t(cosf(t) * r * 1.8f), cy - int16_t(sinf(t) * r * 1.8f), sun);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1, 1 - a * 0.85f * (1 - rise), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("sunrise", scene);
static_assert(face_preset_valid(preset), "Invalid Sunrise face");
} }

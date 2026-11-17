/**
 * @file manga_shock.h
 * @brief Tall blank monochrome eyes framed by a radial manga impact panel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace manga_shock_face {
/** @brief Keep converging speed lines outside the eye region and avoid strobing. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t ink = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 85));
	if (a > 0) {
		for (int i = 0; i < 20; ++i) {
			float angle = i * 0.31415926f;
			float wave = 0.9f + 0.06f * sinf(ms * 0.003f + i);
			int16_t cx = cfg.screenW / 2, cy = cfg.screenH / 2;
			gfx->drawLine(cx + int16_t(cosf(angle) * cfg.screenW * 0.46f),
				cy + int16_t(sinf(angle) * cfg.screenH * 0.46f),
				cx + int16_t(cosf(angle) * cfg.screenW * 0.39f * wave),
				cy + int16_t(sinf(angle) * cfg.screenH * 0.34f * wave), ink);
		}
	}
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.3f, 1 + a * 0.18f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("manga_shock", scene);
static_assert(face_preset_valid(preset), "Invalid Manga Shock face");
} }

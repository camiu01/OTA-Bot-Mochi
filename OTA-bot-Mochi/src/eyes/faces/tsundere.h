/**
 * @file tsundere.h
 * @brief A stubborn sidelong squint softens into a flustered wink with striped blush.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace tsundere_face {
/** @brief Blend the second beat into the first without changing the six-second return. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float soft = (ms % 6000 < 2800) ? 0 : float(ms % 6000 - 2800) / 900;
	soft = soft > 1 ? 1 : soft; soft = soft * soft * (3 - 2 * soft);
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t(a * cfg.screenW / 45 * (1 - 2 * soft));
		float squeeze = b.left ? 0.45f : 0.25f + soft * 0.65f;
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * squeeze, cfg.fgColor);
		if (a <= 0) { continue; }
		uint16_t pink = eye_mix(eye_rgb(250, 120, 160), cfg.bgColor, uint8_t(a * 100));
		int16_t y = b.y + b.h + cfg.screenH / 35;
		for (int i = -1; i <= 1; ++i) {
			int16_t x = b.x + b.w / 2 + i * (cfg.screenW / 45 + 1);
			gfx->drawLine(x, y, x - 3, y + cfg.screenH / 35, pink);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("tsundere", scene);
static_assert(face_preset_valid(preset), "Invalid Tsundere face");
} }

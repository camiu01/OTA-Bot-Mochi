/**
 * @file fireworks.h
 * @brief Wide upturned eyes watch three staggered radial firework bursts.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace fireworks_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each burst expands and fades over 1.8 seconds at its own offset. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * H / 40), 1 + a * 0.05f, 1 + a * 0.05f, cfg.fgColor);
	if (a <= 0) { return true; }
	constexpr uint16_t colors[3] = {eye_rgb(255, 90, 90), eye_rgb(120, 200, 255), eye_rgb(255, 220, 90)};
	for (int k = 0; k < 3; ++k) {
		float grow = ((ms + k * 600) % 1800) / 1800.0f;
		int16_t cx = W / 5 + k * W * 3 / 10, cy = H / 6 + (k & 1) * H / 10, r = int16_t(grow * W / 6) + 2;
		uint16_t c = eye_mix(colors[k], cfg.bgColor, uint8_t(a * 100 * (1 - grow * 0.7f)));
		for (int i = 0; i < 10; ++i) {
			float ang = i * 0.628f;
			gfx->drawLine(cx + int16_t(cosf(ang) * r * 0.6f), cy + int16_t(sinf(ang) * r * 0.6f),
				cx + int16_t(cosf(ang) * r), cy + int16_t(sinf(ang) * r), c);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("fireworks", scene);
static_assert(face_preset_valid(preset), "Invalid Fireworks face");
} }

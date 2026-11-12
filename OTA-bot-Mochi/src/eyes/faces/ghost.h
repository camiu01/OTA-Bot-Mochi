/**
 * @file ghost.h
 * @brief Floating spectral eyes fade, drift and rematerialize.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace ghost_face {
/** @brief Smooth opacity and two trailing wisps distinguish the apparition from ordinary sleepy eyes. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float wave = sinf((ms % 6000) * .0020943951f);
	float opacity = 1 - a * (.25f + .65f * wave * wave);
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t(wave * a * cfg.screenW / 30);
		b.y += int16_t(cosf((ms % 6000) * .0020943951f) * a * cfg.screenH / 30);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - .2f * a, eye_rgb(168, 236, 220), opacity);
		if (a <= 0) { continue; }
		uint16_t ink = eye_mix(eye_rgb(98, 166, 170), cfg.bgColor, uint8_t(a * opacity * 70));
		for (int16_t i : {1, 2}) {
			int16_t x = b.x + b.w * i / 3, y = b.y + b.h;
			gfx->drawLine(x, y, x + int16_t(wave * 5), y + cfg.screenH / 30, ink);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("ghost", scene);
static_assert(face_preset_valid(preset), "Invalid Ghost face");
} }

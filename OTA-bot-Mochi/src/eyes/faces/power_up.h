/**
 * @file power_up.h
 * @brief Shonen charge with rising golden aura strokes around determined eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace power_up_face {
/** @brief Raise a bounded aura without flashing the whole screen or hiding the eyes. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t gold = eye_mix(eye_rgb(255, 197, 56), cfg.bgColor, uint8_t(a * 100));
	for (auto b : {pair.left, pair.right}) {
		float pulse = 0.5f + 0.5f * sinf(ms * 0.009f);
		b.y -= int16_t(a * pulse * cfg.screenH / 100);
		for (int i = 0; i < 5 && a > 0; ++i) {
			int16_t x = b.x + b.w * i / 4, lift = int16_t((ms / 10 + i * 19) % (cfg.screenH / 8));
			int16_t top = b.y - cfg.screenH / 40 - lift;
			gfx->drawLine(x, top, x + (i - 2) * 2, top - cfg.screenH / 14, gold);
			gfx->drawLine(x, b.y + b.h + 3, x, b.y + b.h + cfg.screenH / 18, gold);
		}
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.04f, 1 - a * 0.25f, eye_rgb(255, 240, 165));
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("power_up", scene);
static_assert(face_preset_valid(preset), "Invalid Power Up face");
} }

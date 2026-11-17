/**
 * @file berserk.h
 * @brief Tightening trembling eyes discharge jagged red energy.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace berserk_face {
/** @brief Keep energy local to the eye pair, with no full-screen flashing. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t(sinf((ms % 6000) * 0.045f) * a * cfg.screenW / 80);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * .65f, eye_rgb(255, 70, 46));
		if (a <= 0) { continue; }
		uint16_t energy = eye_mix(eye_rgb(255, 168, 70), cfg.bgColor, uint8_t(a * 100));
		for (uint8_t i = 0; i < 3; ++i) {
			int16_t x = b.x + (i + 1) * b.w / 4, y = b.y - cfg.screenH / 40;
			int16_t reach = int16_t((cfg.screenH / 12 + i * 2) * a);
			int16_t bend = (b.left ? -1 : 1) * cfg.screenW / 40;
			gfx->drawLine(x, y, x + bend, y - reach / 2, energy);
			gfx->drawLine(x + bend, y - reach / 2, x, y - reach, energy);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("berserk", scene);
static_assert(face_preset_valid(preset), "Invalid Berserk face");
} }

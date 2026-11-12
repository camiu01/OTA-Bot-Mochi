/**
 * @file mecha_lock.h
 * @brief Cyan tracking brackets converge around an amber scanning visor.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace mecha_lock_face {
/** @brief Animate external HUD corners and a slit scan, without replacing eyes with armor. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t cyan = eye_mix(eye_rgb(80, 220, 245), cfg.bgColor, uint8_t(a * 100));
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.6f, eye_rgb(255, 179, 64));
		if (a <= 0) { continue; }
		int16_t gap = cfg.screenW / 45 + int16_t((0.5f + 0.5f * cosf(ms * 0.003f)) * cfg.screenW / 40);
		int16_t top = b.y + b.h / 4 - gap, bottom = b.y + b.h * 3 / 4 + gap;
		int16_t len = b.w / 5 + 1;
		for (int sign : {-1, 1}) {
			int16_t x = b.x + b.w / 2 + sign * (b.w / 2 + gap);
			gfx->drawLine(x, top, x - sign * len, top, cyan);
			gfx->drawLine(x, top, x, top + len, cyan);
			gfx->drawLine(x, bottom, x - sign * len, bottom, cyan);
			gfx->drawLine(x, bottom, x, bottom - len, cyan);
		}
		int16_t x = b.x + b.w / 5 + (ms / 35) % (b.w * 3 / 5 + 1);
		gfx->drawLine(x, b.y + b.h * 2 / 5, x, b.y + b.h * 3 / 5, cyan);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("mecha_lock", scene);
static_assert(face_preset_valid(preset), "Invalid Mecha Lock face");
} }

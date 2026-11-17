/**
 * @file turn_signal.h
 * @brief Eyes glance right, then left, beside sequential amber indicator arrows.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace turn_signal_face {
/** @brief Light one to three arrowheads above the outer eye; the glance glides between both sides. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint32_t phase = ms % 6000;
	bool right = phase < 3000;
	float glance = cosf(phase * .000523598776f);
	for (auto b : {pair.left, pair.right}) {
		b.x = eye_limit(b.x + int16_t(glance * a * cfg.screenW / 16), 0, cfg.screenW - b.w);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * (b.left == right ? .3f : .1f), cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t amber = eye_mix(eye_rgb(255, 170, 40), cfg.bgColor, uint8_t(a * 100));
	const EyeBox &e = right ? pair.right : pair.left;
	int16_t s = cfg.screenW / 20 + 1, sign = right ? 1 : -1;
	int16_t y = eye_limit(e.y - s * 2, s + 1, cfg.screenH - s - 2);
	int16_t x = e.x + e.w / 2 - sign * s * 2;
	for (uint8_t i = 0; i < (ms % 800) / 200; ++i, x += sign * s * 2) {
		gfx->fillTriangle(x, y - s, x, y + s, x + sign * s, y, amber);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("turn_signal", scene);
static_assert(face_preset_valid(preset), "Invalid Turn Signal face");
} }

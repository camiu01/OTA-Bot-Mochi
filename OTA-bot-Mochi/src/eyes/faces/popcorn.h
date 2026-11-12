/**
 * @file popcorn.h
 * @brief Squashed eyes jolt as buttery popcorn kernels pop upward on small arcs.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace popcorn_face {
/** @brief Draw a handful of bounded three-lobed puffs, with no screen-filling particles. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		float beat = float((ms + (b.left ? 0 : 400)) % 1100) / 1100;
		b.y += int16_t(a * sinf(beat * 6.2831853f) * cfg.screenH / 80);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.1f, 1 - a * 0.45f, cfg.fgColor);
		if (a <= 0) { continue; }
		uint16_t butter = eye_mix(eye_rgb(255, 225, 142), cfg.bgColor, uint8_t(a * 100));
		for (int i = 0; i < 3; ++i) {
			float t = float((ms + i * 350 + (b.left ? 0 : 170)) % 1500) / 1500;
			int16_t r = cfg.screenW / 85 + 1;
			int16_t x = b.x + b.w / 2 + int16_t((i - 1) * t * cfg.screenW / 18);
			int16_t y = b.y - cfg.screenH / 40 - int16_t(4 * t * (1 - t) * cfg.screenH / 7);
			gfx->fillCircle(x - r, y, r, butter); gfx->fillCircle(x + r, y, r, butter);
			gfx->fillCircle(x, y - r, r + 1, butter);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("popcorn", scene);
static_assert(face_preset_valid(preset), "Invalid Popcorn face");
} }

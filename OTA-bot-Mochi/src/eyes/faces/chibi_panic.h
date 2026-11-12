/**
 * @file chibi_panic.h
 * @brief Tiny trembling eyes beneath a blue gloom comb and flustered motion strokes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace chibi_panic_face {
/** @brief Shrink rather than replace the eyes, keeping a readable panicked pair. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t blue = eye_mix(eye_rgb(88, 120, 235), cfg.bgColor, uint8_t(a * 100));
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t(sinf(ms * 0.025f) * a * cfg.screenW / 100);
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.55f, 1 - a * 0.65f, cfg.fgColor);
		if (a <= 0) { continue; }
		for (int i = 0; i < 4; ++i) {
			int16_t x = b.x + b.w * (i + 1) / 5, y = b.y - cfg.screenH / 15;
			gfx->drawLine(x, y, x, y + cfg.screenH / 12 + (i & 1) * 3, blue);
		}
		int16_t x = b.left ? b.x - 5 : b.x + b.w + 5, y = b.y + b.h / 2;
		gfx->drawLine(x, y - 5, x + (b.left ? -4 : 4), y + 5, blue);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("chibi_panic", scene);
static_assert(face_preset_valid(preset), "Invalid Chibi Panic face");
} }

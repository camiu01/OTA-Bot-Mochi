/**
 * @file ninja.h
 * @brief A tied indigo headband and short lateral afterimages above narrowed eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace ninja_face {
/** @brief Keep the headband outside the eyes and ease the dash back to its starting pose. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	int16_t dash = int16_t(sinf(ms * 0.004f) * a * cfg.screenW / 35);
	uint16_t band = eye_mix(eye_rgb(95, 110, 220), cfg.bgColor, uint8_t(a * 100));
	for (auto b : {pair.left, pair.right}) {
		b.x += dash;
		if (a > 0) {
			for (int i = 1; i <= 3; ++i) {
				uint16_t trail = eye_mix(band, cfg.bgColor, uint8_t(60 - i * 14));
				gfx->drawLine(b.x - i * 4, b.y + b.h / 2 + i * 3, b.x - i * 4 - b.w / 3,
					b.y + b.h / 2 + i * 3, trail);
			}
		}
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.55f, cfg.fgColor);
	}
	if (a > 0) {
		int16_t x = pair.left.x, y = pair.left.y - cfg.screenH / 28, w = pair.right.x + pair.right.w - x;
		gfx->fillRoundRect(x, y, w, cfg.screenH / 35 + 2, 2, band);
		int16_t cx = cfg.screenW / 2, r = cfg.screenW / 45 + 1;
		gfx->drawRect(cx - r, y - 1, r * 2, cfg.screenH / 35 + 4, eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 100)));
		gfx->drawLine(x + w, y + 2, x + w + cfg.screenW / 15, y - cfg.screenH / 25 + dash, band);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("ninja", scene);
static_assert(face_preset_valid(preset), "Invalid Ninja face");
} }

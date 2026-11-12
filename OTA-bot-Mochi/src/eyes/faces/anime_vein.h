/**
 * @file anime_vein.h
 * @brief A pulsing four-pronged anger mark over taut, compressed eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace anime_vein_face {
/** @brief Draw the classic disconnected brow vein, not a cross-shaped pupil. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(sinf(ms * 0.014f) * a * 1.5f);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.05f, 1 - a * 0.5f, cfg.fgColor);
	}
	if (a > 0) {
		int16_t cx = pair.right.x + pair.right.w / 2, cy = pair.right.y - cfg.screenH / 18;
		int16_t r = cfg.screenW / 35 + 1 + int16_t((1 + sinf(ms * 0.01f)) * a * 2);
		uint16_t red = eye_mix(eye_rgb(255, 65, 90), cfg.bgColor, uint8_t(a * 100));
		for (int sx : {-1, 1}) {
			for (int sy : {-1, 1}) {
				gfx->drawLine(cx + sx * r, cy + sy * 2, cx + sx * 3, cy + sy * 3, red);
				gfx->drawLine(cx + sx * 3, cy + sy * 3, cx + sx * 2, cy + sy * r, red);
			}
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("anime_vein", scene);
static_assert(face_preset_valid(preset), "Invalid Anime Vein face");
} }

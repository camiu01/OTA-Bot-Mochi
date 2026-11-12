/**
 * @file rubber_band.h
 * @brief Elastic eyes pull apart, flatten and rebound with a damped wobble.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace rubber_band_face {
/** @brief Stretch the actual pair without introducing symbols or extra pupils. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float pull = 0.5f + 0.5f * sinf((ms % 6000) * 0.00418879f);
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t((b.left ? -1 : 1) * a * pull * cfg.screenW / 45);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * pull * 0.3f,
			1 - a * (0.25f + pull * 0.45f), cfg.fgColor);
		if (a > 0) {
			uint16_t ink = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 70));
			int16_t y = b.y + b.h / 2, x = b.left ? b.x - 4 : b.x + b.w + 4;
			int16_t sign = b.left ? -1 : 1;
			gfx->drawLine(x, y - 5, x + sign * 5, y - 8, ink);
			gfx->drawLine(x, y + 5, x + sign * 5, y + 8, ink);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("rubber_band", scene);
static_assert(face_preset_valid(preset), "Invalid Rubber Band face");
} }

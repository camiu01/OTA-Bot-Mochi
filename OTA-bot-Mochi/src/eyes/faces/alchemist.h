/**
 * @file alchemist.h
 * @brief Rotating transmutation circles surrounding calm cyan eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace alchemist_face {
/** @brief Draw a double ring and connected triangular sigil outside each eye. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint16_t ink = eye_mix(eye_rgb(72, 225, 240), cfg.bgColor, uint8_t(a * 100));
	for (const auto &b : {pair.left, pair.right}) {
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, r = (b.h > b.w ? b.h : b.w) / 2 + cfg.screenW / 35;
		if (a > 0) {
			gfx->drawCircle(cx, cy, r, ink); gfx->drawCircle(cx, cy, r + 3, ink);
			for (int i = 0; i < 3; ++i) {
				float angle = ms * 0.0007f + i * 2.0943951f;
				float next = angle + 2.0943951f;
				gfx->drawLine(cx + int16_t(cosf(angle) * r), cy + int16_t(sinf(angle) * r),
					cx + int16_t(cosf(next) * r), cy + int16_t(sinf(next) * r), ink);
			}
		}
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.12f, 1 - a * 0.12f, eye_rgb(145, 240, 235));
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("alchemist", scene);
static_assert(face_preset_valid(preset), "Invalid Alchemist face");
} }

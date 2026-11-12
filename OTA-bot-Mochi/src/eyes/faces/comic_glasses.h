/**
 * @file comic_glasses.h
 * @brief Oversized purple glasses slip unevenly down the eyes and wobble back.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace comic_glasses_face {
/** @brief Keep both real eyes visible inside hollow frames, with no replacement pupils. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.15f, 1 - a * 0.2f, cfg.fgColor);
	}
	if (a > 0) {
		uint16_t purple = eye_mix(eye_rgb(195, 115, 235), cfg.bgColor, uint8_t(a * 100));
		int16_t pad = cfg.screenW / 35 + 1, bridge_y[2] = {};
		int i = 0;
		for (const auto &b : {pair.left, pair.right}) {
			int16_t slip = int16_t(a * (0.5f + 0.5f * sinf(ms * 0.003f + i * 1.5f)) * cfg.screenH / 35);
			int16_t y = b.y - pad + slip; bridge_y[i++] = y + b.h / 3;
			for (int line = 0; line < cfg.screenW / 100 + 1; ++line) {
				gfx->drawRoundRect(b.x - pad + line, y + line, b.w + pad * 2 - line * 2,
					b.h + pad * 2 - line * 2, pad * 2, purple);
			}
		}
		gfx->drawLine(pair.left.x + pair.left.w + pad, bridge_y[0],
			pair.right.x - pad, bridge_y[1], purple);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("comic_glasses", scene);
static_assert(face_preset_valid(preset), "Invalid Comic Glasses face");
} }

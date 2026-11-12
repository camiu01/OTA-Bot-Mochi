/**
 * @file sweat_drop.h
 * @brief Uneasy asymmetric eyes with an oversized sliding anime sweat drop.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace sweat_drop_face {
/** @brief Let the drop creep down the outer temple while the eyes hold an awkward squint. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * (b.left ? 0.48f : 0.2f), cfg.fgColor);
	}
	if (a > 0) {
		int16_t r = cfg.screenW / 35 + 1;
		int16_t x = pair.right.x + pair.right.w + r + 3;
		x = eye_limit(x, r, cfg.screenW - r - 1);
		int16_t y = pair.right.y + int16_t((ms % 6000) * cfg.screenH / 60000);
		uint16_t blue = eye_mix(eye_rgb(75, 185, 255), cfg.bgColor, uint8_t(a * 100));
		gfx->fillTriangle(x, y - r * 2, x - r, y, x + r, y, blue);
		gfx->fillCircle(x, y, r, blue);
		gfx->drawLine(x - r / 3, y - r / 2, x - r / 3, y + r / 3,
			eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 85)));
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("sweat_drop", scene);
static_assert(face_preset_valid(preset), "Invalid Sweat Drop face");
} }

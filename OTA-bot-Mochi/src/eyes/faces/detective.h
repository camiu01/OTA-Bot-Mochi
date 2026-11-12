/**
 * @file detective.h
 * @brief A moving magnifying glass searches, then locks onto a detail.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace detective_face {
/** @brief Keep both real eyes, with an external lens and handle rather than an added pupil. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	const auto &b = pair.right;
	int16_t r = eye_limit(b.w * 3 / 4, 3, cfg.screenW / 8);
	int16_t sweep = int16_t(sinf((ms % 6000) * .00314159265f) * a * b.w / 5);
	int16_t cx = b.x + b.w / 2 + sweep, cy = b.y + b.h / 2;
	uint16_t rim = eye_mix(eye_rgb(92, 204, 240), cfg.bgColor, uint8_t(a * 100));
	gfx->drawCircle(cx, cy, r, rim); gfx->drawCircle(cx, cy, r + 1, rim);
	gfx->drawLine(cx + r * 2 / 3, cy + r * 2 / 3, cx + r * 4 / 3, cy + r * 4 / 3, rim);
	gfx->drawLine(cx - r / 2, cy - r / 2, cx - r / 4, cy - r * 2 / 3, rim);
	if (ms % 6000 >= 3000) {
		int16_t x = cx - r / 3, y = cy + r / 3;
		gfx->drawLine(x - 2, y - 2, x, y, rim); gfx->drawLine(x, y, x + 4, y - 5, rim);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("detective", scene);
static_assert(face_preset_valid(preset), "Invalid Detective face");
} }

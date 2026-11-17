/**
 * @file dot_eyes.h
 * @brief Speechless eyes collapse to tiny dots while a three-dot ellipsis types out.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace dot_eyes_face {
/** @brief Shrink the real pair around its centers; the ellipsis counts in and resets. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float side = float(cfg.screenW / 22 + 3);
	for (const auto &b : {pair.left, pair.right}) {
		float sx = 1 - a * (1 - side / b.w), sy = 1 - a * (1 - side / b.h);
		face_cinematics::body(gfx, cfg, b, a, sx, sy, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t ink = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 85));
	int16_t r = cfg.screenW / 36 + 2, gap = r * 4;
	int16_t x = cfg.screenW / 2 - gap;
	int16_t y = eye_limit(pair.left.y + pair.left.h + cfg.screenH / 8, r, cfg.screenH - r - 1);
	uint8_t dots = uint8_t((ms / 700) % 4);
	for (uint8_t i = 0; i < dots; ++i) { gfx->fillCircle(x + i * gap, y, r, ink); }
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("dot_eyes", scene);
static_assert(face_preset_valid(preset), "Invalid Dot Eyes face");
} }

/**
 * @file penguin.h
 * @brief Waddling eyes between flapping navy flippers above stepping orange feet.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace penguin_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Feet alternate with the waddle while the flippers swing. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float waddle = sinf(ms * 0.0075f);
	if (a > 0) {
		uint16_t navy = tint(cfg, eye_rgb(40, 55, 90), a), orange = tint(cfg, eye_rgb(250, 150, 40), a);
		int16_t cy = pair.left.y + pair.left.h / 2, f = H / 8 + 2;
		int16_t lx = pair.left.x - W / 20, rx = pair.right.x + pair.right.w + W / 20;
		gfx->fillTriangle(lx, cy - f, lx, cy + f / 2, lx - W / 12, cy + f + int16_t(waddle * f / 2), navy);
		gfx->fillTriangle(rx, cy - f, rx, cy + f / 2, rx + W / 12, cy + f - int16_t(waddle * f / 2), navy);
		int16_t fy = H - H / 14;
		gfx->fillEllipse(W / 2 - W / 10, fy - int16_t(waddle > 0 ? waddle * 4 : 0), W / 20 + 2, H / 50 + 2, orange);
		gfx->fillEllipse(W / 2 + W / 10, fy - int16_t(waddle < 0 ? -waddle * 4 : 0), W / 20 + 2, H / 50 + 2, orange);
	}
	pair_body(gfx, cfg, pair, a, int16_t(waddle * a * W / 40), 0, 1, 1 - a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("penguin", scene);
static_assert(face_preset_valid(preset), "Invalid Penguin face");
} }

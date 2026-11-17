/**
 * @file dragon_egg.h
 * @brief Curious eyes watch a speckled green egg wobble harder, crack and puff a spark.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace dragon_egg_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Cracks spread over four seconds; then a small ember rises from the top. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 6000;
	float crack = t < 4000 ? t / 4000.0f : 1, wob = sinf(ms * (0.01f + crack * 0.02f)) * crack;
	if (a > 0) {
		uint16_t shell = tint(cfg, eye_rgb(120, 200, 150), a), spot = tint(cfg, eye_rgb(60, 130, 90), a);
		uint16_t seam = tint(cfg, eye_rgb(30, 50, 40), a);
		int16_t rx = W / 12 + 3, ry = rx + rx / 3, cx = W / 2 + int16_t(wob * 4), cy = H - ry - H / 20, n = int16_t(crack * 4);
		gfx->fillEllipse(cx, cy, rx, ry, shell);
		gfx->fillCircle(cx - rx / 3, cy - ry / 3, 2, spot);
		gfx->fillCircle(cx + rx / 3, cy + ry / 4, 3, spot);
		for (int16_t i = 0; i < n; ++i) {
			gfx->drawLine(cx - rx + i * rx / 2, cy - (i & 1) * 4, cx - rx + (i + 1) * rx / 2, cy - ((i + 1) & 1) * 4, seam);
		}
		if (t >= 4000) { gfx->fillCircle(cx, cy - ry - 4 - int16_t((t - 4000) / 60 % 10), 3, tint(cfg, eye_rgb(255, 120, 40), a)); }
	}
	pair_body(gfx, cfg, pair, a, int16_t(wob * a * 3), int16_t(a * H / 40), 1 + a * 0.1f, 1 + a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("dragon_egg", scene);
static_assert(face_preset_valid(preset), "Invalid Dragon Egg face");
} }

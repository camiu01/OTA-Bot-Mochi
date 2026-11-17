/**
 * @file kawaii_wink.h
 * @brief A blushing wink: the right eye closes and a gold star bounces away from it.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace kawaii_wink_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The star leaves the closed eye every 1.6 seconds along hopping arcs. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float p = (ms % 1600) / 1600.0f;
	for (auto b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, b.left ? 1 + a * 0.05f : 1 - a * 0.9f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t star = tint(cfg, eye_rgb(255, 225, 80), a), blush = tint(cfg, eye_rgb(255, 150, 175), a);
	for (auto b : {pair.left, pair.right}) { gfx->fillEllipse(b.x + b.w / 2, b.y + b.h + H / 30, b.w / 3, H / 60 + 2, blush); }
	int16_t sx = pair.right.x + pair.right.w + int16_t(p * W / 5);
	int16_t sy = pair.right.y + pair.right.h / 2 - int16_t(fabsf(sinf(p * 6.28f)) * H / 8), s = W / 24 + 3;
	gfx->fillTriangle(sx - s, sy, sx + s, sy, sx, sy - s * 2, star);
	gfx->fillTriangle(sx - s, sy - s * 4 / 3, sx + s, sy - s * 4 / 3, sx, sy + s * 2 / 3, star);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("kawaii_wink", scene);
static_assert(face_preset_valid(preset), "Invalid Kawaii Wink face");
} }

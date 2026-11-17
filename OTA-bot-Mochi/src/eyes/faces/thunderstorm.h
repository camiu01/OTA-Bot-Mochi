/**
 * @file thunderstorm.h
 * @brief Eyes flinch shut whenever a zigzag bolt drops from a drifting storm cloud.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace thunderstorm_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief A short bolt appears once per 1.3 seconds, without a full-screen flash. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	bool strike = (ms % 1300) < 260;
	pair_body(gfx, cfg, pair, a, 0, 0, 1, 1 - a * (strike ? 0.75f : 0.2f), cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t cloud = tint(cfg, eye_rgb(80, 85, 105), a), bolt = tint(cfg, eye_rgb(255, 240, 90), a);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t r = W / 16 + 2, cx = W / 2 + int16_t(sinf(ms * 0.002f) * W / 12);
	int16_t cy = eye_limit(top - H / 7, r + 1, H - 1);
	gfx->fillCircle(cx - r, cy, r, cloud);
	gfx->fillCircle(cx + r, cy, r, cloud);
	gfx->fillCircle(cx, cy - r / 2, r + r / 3, cloud);
	if (!strike) { return true; }
	int16_t s = H / 30 + 2, y = cy + r;
	for (int k = 0; k < 2; ++k) {
		gfx->drawLine(cx + k, y, cx - s + k, y + s, bolt);
		gfx->drawLine(cx - s + k, y + s, cx + k, y + s, bolt);
		gfx->drawLine(cx + k, y + s, cx - s + k, y + s * 2, bolt);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("thunderstorm", scene);
static_assert(face_preset_valid(preset), "Invalid Thunderstorm face");
} }

/**
 * @file disco.h
 * @brief Bouncing eyes under a turning mirror ball while colored light spots roam.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace disco_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Eight spots orbit on different paths; the ball's seams slide as it turns. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t bob = int16_t(fabsf(sinf(ms * 0.012f)) * a * H / 50);
	if (a > 0) {
		constexpr uint16_t lights[4] = {eye_rgb(255, 80, 160), eye_rgb(80, 220, 255), eye_rgb(255, 230, 70),
			eye_rgb(130, 255, 120)};
		for (int i = 0; i < 8; ++i) {
			float t = ms * 0.002f + i * 0.785f;
			int16_t x = W / 2 + int16_t(cosf(t) * W * (0.25f + 0.05f * (i & 1))), y = H / 2 + int16_t(sinf(t * 1.3f) * H * 0.35f);
			gfx->fillCircle(x, y, W / 50 + 2, tint(cfg, lights[i & 3], a));
		}
		uint16_t silver = tint(cfg, eye_rgb(200, 205, 215), a), seam = tint(cfg, eye_rgb(90, 95, 110), a);
		int16_t r = W / 14 + 2, cy = r + 2, slide = int16_t((ms / 80) % (r / 2 + 1));
		gfx->fillCircle(W / 2, cy, r, silver);
		for (int k = -1; k <= 1; ++k) {
			gfx->drawFastHLine(W / 2 - r, cy + k * r / 2, r * 2, seam);
			gfx->drawFastVLine(W / 2 + k * r / 2 + slide - r / 4, cy - r, r * 2, seam);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, -bob, 1, 1 - a * 0.25f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("disco", scene);
static_assert(face_preset_valid(preset), "Invalid Disco face");
} }

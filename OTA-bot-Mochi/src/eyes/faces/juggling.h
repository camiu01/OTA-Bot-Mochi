/**
 * @file juggling.h
 * @brief Eyes sway beneath three colored balls juggled in a cascade pattern.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace juggling_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each ball crosses once per 1.2 seconds, alternating direction. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	pair_body(gfx, cfg, pair, a, int16_t(sinf(ms * 0.006f) * a * W / 60), 0, 1, 1 - a * 0.2f, cfg.fgColor);
	if (a <= 0) { return true; }
	constexpr uint16_t colors[3] = {eye_rgb(255, 80, 80), eye_rgb(80, 180, 255), eye_rgb(255, 210, 60)};
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	for (int i = 0; i < 3; ++i) {
		uint32_t clock = ms + i * 400;
		float p = (clock % 1200) / 1200.0f, u = ((clock / 1200 + i) & 1) ? p : 1 - p;
		int16_t x = int16_t(W / 4 + u * W / 2), y = int16_t(top - H / 20 - sinf(p * 3.14159f) * H / 4);
		gfx->fillCircle(x, y, W / 40 + 2, tint(cfg, colors[i], a));
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("juggling", scene);
static_assert(face_preset_valid(preset), "Invalid Juggling face");
} }

/**
 * @file tornado.h
 * @brief Wobbling eyes are tugged sideways toward a swaying grey tornado funnel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace tornado_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Seven stacked strokes form the funnel; each sways with its own phase. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t cx = W / 2 + int16_t(sinf(ms * 0.004f) * W / 6);
	for (auto b : {pair.left, pair.right}) {
		b.x += int16_t((cx - (b.x + b.w / 2)) * 0.12f * a + sinf(ms * 0.03f) * a * 1.5f);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.25f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t grey = tint(cfg, eye_rgb(150, 150, 165), a);
	for (int k = 0; k < 7; ++k) {
		int16_t y = H - 4 - k * H / 16, rx = W / 30 + k * W / 28;
		int16_t x = cx + int16_t(sinf(ms * 0.01f + k) * W / 40) - rx;
		gfx->drawFastHLine(x, y, rx * 2, grey);
		gfx->drawFastHLine(x + 2, y + 2, rx * 2 - 4, grey);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("tornado", scene);
static_assert(face_preset_valid(preset), "Invalid Tornado face");
} }

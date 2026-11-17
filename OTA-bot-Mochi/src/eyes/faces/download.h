/**
 * @file download.h
 * @brief Eyes watch a green progress bar fill under a bouncing arrow, then smile.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace download_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The bar fills in five seconds; completion turns the pair into a squint. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float p = (ms % 6000) / 5000.0f;
	p = p > 1 ? 1 : p;
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * H / 40), 1, 1 - a * (p >= 1 ? 0.7f : 0.2f), cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t frame = tint(cfg, eye_rgb(150, 160, 180), a), fill = tint(cfg, eye_rgb(90, 220, 120), a);
	int16_t w = W * 2 / 3, x = W / 6, y = H - H / 8, h = H / 25 + 3, s = W / 30 + 2;
	gfx->drawRect(x, y, w, h, frame);
	gfx->fillRect(x + 2, y + 2, int16_t((w - 4) * p), h - 4, fill);
	int16_t ay = y - H / 8 + int16_t((ms / 30) % (H / 20 + 1));
	gfx->fillRect(W / 2 - s / 3, ay - s * 2, s * 2 / 3 + 1, s * 2, fill);
	gfx->fillTriangle(W / 2 - s, ay, W / 2 + s, ay, W / 2, ay + s, fill);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("download", scene);
static_assert(face_preset_valid(preset), "Invalid Download face");
} }

/**
 * @file dj.h
 * @brief Eyes nod on the beat under a headphone band, with bouncing equalizer bars.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace dj_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The pair dips at the start of every 500 ms beat. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t bob = (ms % 500) < 120 ? int16_t(a * 3) : 0;
	if (a > 0) {
		uint16_t band = tint(cfg, eye_rgb(60, 60, 75), a), cup = tint(cfg, eye_rgb(230, 60, 120), a);
		uint16_t bar = tint(cfg, eye_rgb(80, 220, 200), a);
		int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 2, r = W * 2 / 5;
		for (int k = 0; k < 3; ++k) {
			for (int i = 0; i < 16; ++i) {
				float t0 = 3.14159f * i / 16, t1 = 3.14159f * (i + 1) / 16;
				gfx->drawLine(cx + int16_t(cosf(t0) * (r + k)), cy - int16_t(sinf(t0) * (r + k)),
					cx + int16_t(cosf(t1) * (r + k)), cy - int16_t(sinf(t1) * (r + k)), band);
			}
		}
		gfx->fillRoundRect(cx - r - W / 30, cy - H / 16, W / 15, H / 8, 3, cup);
		gfx->fillRoundRect(cx + r - W / 30, cy - H / 16, W / 15, H / 8, 3, cup);
		for (int i = 0; i < 6; ++i) {
			int16_t h = 3 + int16_t(fabsf(sinf(ms * 0.01f + i * 1.7f)) * H / 10);
			gfx->fillRect(W / 4 + i * W / 12, H - 4 - h, W / 18, h, bar);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, bob, 1, 1 - a * 0.4f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("dj", scene);
static_assert(face_preset_valid(preset), "Invalid Dj face");
} }

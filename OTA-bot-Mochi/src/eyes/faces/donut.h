/**
 * @file donut.h
 * @brief Eyes track a pink-frosted donut that glides back and forth with spinning sprinkles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace donut_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Sprinkles rotate around the ring; the pair leans toward the donut. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t cx = W / 2 + int16_t(cosf(ms * 0.003f) * W / 4), cy = H - H / 6;
	if (a > 0) {
		uint16_t dough = tint(cfg, eye_rgb(215, 160, 95), a), icing = tint(cfg, eye_rgb(255, 130, 185), a);
		int16_t r = W / 12 + 3;
		gfx->fillCircle(cx, cy, r, dough);
		gfx->fillCircle(cx, cy, r - 2, icing);
		gfx->fillCircle(cx, cy, r / 3, cfg.bgColor);
		for (int i = 0; i < 6; ++i) {
			float t = i * 1.047f + ms * 0.004f;
			uint16_t c = tint(cfg, i & 1 ? eye_rgb(255, 255, 255) : eye_rgb(110, 210, 255), a);
			int16_t sx = cx + int16_t(cosf(t) * r * 0.66f), sy = cy + int16_t(sinf(t) * r * 0.66f);
			gfx->drawLine(sx, sy, sx + 2, sy + 1, c);
		}
	}
	pair_body(gfx, cfg, pair, a, int16_t((cx - W / 2) * 0.15f * a), int16_t(a * H / 40), 1, 1 - a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("donut", scene);
static_assert(face_preset_valid(preset), "Invalid Donut face");
} }

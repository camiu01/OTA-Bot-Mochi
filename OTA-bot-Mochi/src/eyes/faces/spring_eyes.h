/**
 * @file spring_eyes.h
 * @brief Eyes bounce on visible accordion springs, compressing before each hop.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace spring_eyes_face {
/** @brief Anchor each coil below the eye and retain a continuous six-second recovery. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		float hop = 0.5f + 0.5f * sinf(ms * 0.005f + (b.left ? 0 : 1.3f));
		int16_t anchor = b.y + b.h + cfg.screenH / 18;
		b.y -= int16_t(a * hop * cfg.screenH / 14);
		if (a > 0) {
			uint16_t steel = eye_mix(eye_rgb(145, 190, 205), cfg.bgColor, uint8_t(a * 100));
			int16_t cx = b.x + b.w / 2, top = b.y + b.h - int16_t(a * b.h / 10);
			int16_t radius = cfg.screenW / 45 + 1, px = cx, py = top;
			for (int i = 1; i <= 8; ++i) {
				int16_t x = i == 8 ? cx : cx + ((i & 1) ? radius : -radius);
				int16_t y = top + (anchor - top) * i / 8;
				gfx->drawLine(px, py, x, y, steel); px = x; py = y;
			}
			gfx->drawLine(cx - radius, anchor, cx + radius, anchor, steel);
		}
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * (1 - hop) * 0.3f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("spring_eyes", scene);
static_assert(face_preset_valid(preset), "Invalid Spring Eyes face");
} }

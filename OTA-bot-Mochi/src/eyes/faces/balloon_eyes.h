/**
 * @file balloon_eyes.h
 * @brief Eyes inflate into two tethered pastel balloons and gently deflate.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace balloon_eyes_face {
/** @brief Float the eye bodies above curved strings, preserving the original resting palette. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		float sway = sinf(ms * 0.0025f + (b.left ? 0 : 1));
		b.x += int16_t(a * sway * cfg.screenW / 70);
		b.y -= int16_t(a * cfg.screenH / 20);
		b.radius = int16_t(b.radius * (1 - a) + b.w * 0.625f * a);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.25f, 1 + a * 0.08f,
			b.left ? eye_rgb(255, 145, 190) : eye_rgb(115, 220, 245));
		if (a <= 0) { continue; }
		uint16_t string = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 65));
		int16_t cx = b.x + b.w / 2, top = b.y + b.h + int16_t(a * b.h * 0.04f);
		int16_t px = cx, py = top;
		for (int i = 1; i <= 6; ++i) {
			int16_t x = cx + int16_t(sinf(i * 0.8f + ms * 0.002f) * a * cfg.screenW / 80);
			int16_t y = top + i * cfg.screenH / 90;
			gfx->drawLine(px, py, x, y, string); px = x; py = y;
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("balloon_eyes", scene);
static_assert(face_preset_valid(preset), "Invalid Balloon Eyes face");
} }

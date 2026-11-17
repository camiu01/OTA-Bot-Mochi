/**
 * @file yandere.h
 * @brief Sweet squeezed eyes snap wide open under trembling violet shadow lines.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace yandere_face {
/** @brief Hold a happy squint, then open wide and let the shadow fall over the upper lids. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint32_t t = ms % 6000;
	float wide = t < 2500 ? 0 : float(t - 2500) / 350;
	wide = wide > 1 ? 1 : wide;
	int16_t shake = int16_t(sinf(ms * 0.05f) * wide * a * 1.5f);
	for (auto b : {pair.left, pair.right}) {
		b.x += shake;
		b.y -= int16_t(a * (1 - wide) * cfg.screenH / 40);
		float sx = 1 - a * wide * 0.12f, sy = 1 - a * (0.82f - wide * 0.97f);
		face_cinematics::body(gfx, cfg, b, a, sx, sy, cfg.fgColor);
		if (a <= 0 || wide <= 0) { continue; }
		uint16_t shade = eye_mix(eye_rgb(115, 40, 150), cfg.bgColor, uint8_t(a * wide * 100));
		int16_t bottom = b.y + b.h / 3, step = cfg.screenW / 40 + 2;
		for (int16_t x = b.x - step; x <= b.x + b.w + step; x += step) {
			int16_t top = b.y - cfg.screenH / 10 - ((x / step) & 1) * cfg.screenH / 30;
			top = eye_limit(top, 0, cfg.screenH - 1);
			gfx->drawFastVLine(x, top, bottom - top, shade);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("yandere", scene);
static_assert(face_preset_valid(preset), "Invalid Yandere face");
} }

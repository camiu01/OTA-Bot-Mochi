/**
 * @file car_wash.h
 * @brief A striped roller brush crosses squeezed-shut eyes among rising foam bubbles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace car_wash_face {
/** @brief Each eye closes only while the roller is over it; the roller parks off-panel between passes. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float sweep = .5f - .5f * cosf((ms % 6000) * .0020943951f);
	int16_t bw = cfg.screenW / 7 + 2, bx = int16_t(sweep * (cfg.screenW + bw)) - bw;
	for (const auto &b : {pair.left, pair.right}) {
		int16_t d = b.x + b.w / 2 - bx - bw / 2;
		if (d < 0) { d = -d; }
		float near = d < b.w ? 1 - float(d) / b.w : 0;
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * (.15f + .7f * near), cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t foam = eye_mix(eye_rgb(214, 240, 255), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 7; ++i) {
		float t = float((ms + i * 310) % 2100) / 2100;
		int16_t x = int16_t(((i * 41 + 17) % 100) * cfg.screenW / 100) + int16_t(sinf(t * 6.2831853f + i) * cfg.screenW / 40);
		gfx->drawCircle(x, cfg.screenH - int16_t(t * cfg.screenH), cfg.screenW / 70 + 1 + i % 3, foam);
	}
	uint16_t blue = eye_mix(eye_rgb(52, 132, 255), cfg.bgColor, uint8_t(a * 100));
	uint16_t red = eye_mix(eye_rgb(255, 84, 96), cfg.bgColor, uint8_t(a * 100));
	int16_t band = cfg.screenH / 14 + 1;
	for (int16_t y = 0; y < cfg.screenH; y += band) {
		gfx->fillRect(bx, y, bw, band, ((y / band + ms / 150) & 1) ? blue : red);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("car_wash", scene);
static_assert(face_preset_valid(preset), "Invalid Car Wash face");
} }

/**
 * @file astronaut.h
 * @brief Weightless eyes float inside a round helmet visor while stars drift past.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace astronaut_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief A visor glare sweeps every three seconds; the pair bobs slowly. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float floaty = sinf(ms * 0.002f);
	if (a > 0) {
		uint16_t dot = tint(cfg, eye_rgb(230, 230, 255), a);
		for (int i = 0; i < 8; ++i) {
			gfx->fillRect(int16_t((i * 71 + int(ms / 40)) % W), int16_t((i * 43 + 7) % H), 2, 2, dot);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(floaty * a * H / 40), 1, 1, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t helmet = tint(cfg, eye_rgb(200, 205, 215), a), glare = tint(cfg, eye_rgb(150, 220, 255), a);
	int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 2 + int16_t(floaty * H / 40), r = (W < H ? W : H) * 9 / 20;
	for (int k = 0; k < W / 80 + 2; ++k) { gfx->drawCircle(cx, cy, r - k, helmet); }
	float g = 3.6f + (ms % 3000) / 3000.0f * 1.2f;
	gfx->drawLine(cx + int16_t(cosf(g) * r * 0.8f), cy + int16_t(sinf(g) * r * 0.8f),
		cx + int16_t(cosf(g + 0.4f) * r * 0.8f), cy + int16_t(sinf(g + 0.4f) * r * 0.8f), glare);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("astronaut", scene);
static_assert(face_preset_valid(preset), "Invalid Astronaut face");
} }

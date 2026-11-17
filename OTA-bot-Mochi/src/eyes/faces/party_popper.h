/**
 * @file party_popper.h
 * @brief Delighted wide eyes as a gold party popper sprays curling streamers.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace party_popper_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Streamers extend over two seconds, then burst again. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float burst = (ms % 2000) / 2000.0f;
	if (a > 0) {
		constexpr uint16_t colors[3] = {eye_rgb(255, 90, 140), eye_rgb(90, 200, 255), eye_rgb(140, 230, 90)};
		uint16_t cone = tint(cfg, eye_rgb(240, 190, 60), a);
		int16_t bx = W / 8, by = H - H / 8, s = W / 14 + 2, ox = bx + s / 2, oy = by - s / 2;
		gfx->fillTriangle(bx - s, by + s, bx, by - s, bx + s, by, cone);
		for (int i = 0; i < 6; ++i) {
			float ang = -1.4f + i * 0.24f, len = burst * W * 0.6f;
			uint16_t c = tint(cfg, colors[i % 3], a);
			int16_t px = ox, py = oy;
			for (int k = 1; k <= 4; ++k) {
				float d = len * k / 4;
				int16_t nx = ox + int16_t(cosf(ang) * d + sinf(k * 1.7f + i) * 4), ny = oy + int16_t(sinf(ang) * d);
				gfx->drawLine(px, py, nx, ny, c);
				px = nx; py = ny;
			}
		}
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 + a * 0.08f, 1 + a * (0.1f + 0.08f * burst), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("party_popper", scene);
static_assert(face_preset_valid(preset), "Invalid Party Popper face");
} }

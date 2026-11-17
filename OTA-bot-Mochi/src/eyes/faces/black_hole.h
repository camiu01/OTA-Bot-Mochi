/**
 * @file black_hole.h
 * @brief Stretched eyes are pulled inward toward a spinning violet-and-orange accretion swirl.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace black_hole_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Three spiral arms rotate around a dark core; the pull pulses gently. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float spin = ms * 0.005f;
	int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 2;
	if (a > 0) {
		uint16_t glow = tint(cfg, eye_rgb(255, 140, 60), a), violet = tint(cfg, eye_rgb(150, 70, 220), a);
		int16_t r = W / 7 + 3;
		for (int arm = 0; arm < 3; ++arm) {
			for (int i = 0; i < 10; ++i) {
				float ang = spin + arm * 2.094f + i * 0.35f, rad = r * (0.4f + i * 0.12f) * H / W;
				gfx->fillCircle(cx + int16_t(cosf(ang) * rad), cy + int16_t(sinf(ang) * rad * 0.6f), i > 5 ? 3 : 2, i & 1 ? glow : violet);
			}
		}
		gfx->fillCircle(cx, cy, r / 3 + 1, tint(cfg, eye_rgb(10, 0, 20), a));
	}
	for (auto b : {pair.left, pair.right}) {
		int16_t pull = int16_t(a * W / 30 * (0.8f + 0.2f * sinf(spin)));
		b.x += b.left ? pull : -pull;
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.2f, 1 - a * 0.3f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("black_hole", scene);
static_assert(face_preset_valid(preset), "Invalid Black Hole face");
} }

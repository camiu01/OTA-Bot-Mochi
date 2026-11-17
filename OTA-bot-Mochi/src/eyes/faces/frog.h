/**
 * @file frog.h
 * @brief Bobbing eyes perched on green frog mounds while a tongue flicks out.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace frog_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The tongue shoots out and back once every 1.7 seconds. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float tongue = (ms % 1700) < 400 ? (ms % 1700) / 400.0f : 0;
	for (auto b : {pair.left, pair.right}) {
		if (a > 0) {
			gfx->fillCircle(b.x + b.w / 2, b.y + b.h / 2, b.w / 2 + W / 30, tint(cfg, eye_rgb(90, 190, 80), a));
		}
		b.y -= int16_t(a * (H / 30 + sinf(ms * 0.005f) * 2));
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.2f, 1 - a * 0.35f, cfg.fgColor);
	}
	if (a <= 0 || tongue <= 0) { return true; }
	uint16_t red = tint(cfg, eye_rgb(240, 90, 110), a);
	int16_t y = pair.left.y + pair.left.h + H / 12, x = W / 2 + int16_t(tongue * W / 3);
	int16_t ty = y - int16_t(tongue * H / 10);
	gfx->drawLine(W / 2, y, x, ty, red);
	gfx->fillCircle(x, ty, 3, red);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("frog", scene);
static_assert(face_preset_valid(preset), "Invalid Frog face");
} }

/**
 * @file frosty.h
 * @brief Shivering, ice-tinted eyes grow icicles that hang from their lower edges.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace frosty_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Icicles lengthen during the first half; the shiver continues throughout. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float grow = (ms % 6000) / 3000.0f;
	grow = grow > 1 ? 1 : grow;
	int16_t shiver = int16_t(sinf(ms * 0.07f) * a * 1.5f), hw = W / 80 + 2;
	for (auto b : {pair.left, pair.right}) {
		b.x += shiver;
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.15f, eye_rgb(170, 225, 255));
		if (a <= 0) { continue; }
		uint16_t ice = tint(cfg, eye_rgb(160, 230, 255), a);
		for (int i = 0; i < 4; ++i) {
			int16_t x = b.x + b.w * (i + 1) / 5, y = b.y + b.h;
			int16_t len = int16_t((H / 14 + (i % 2) * H / 30) * grow) + 2;
			gfx->fillTriangle(x - hw, y, x + hw, y, x, y + len, ice);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("frosty", scene);
static_assert(face_preset_valid(preset), "Invalid Frosty face");
} }

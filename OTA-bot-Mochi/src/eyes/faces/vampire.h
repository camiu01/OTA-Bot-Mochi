/**
 * @file vampire.h
 * @brief Red-tinted, narrowed eyes above a high crimson collar while a bat circles overhead.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace vampire_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The bat flaps every 90 ms along a looping path. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float t = ms * 0.004f;
	if (a > 0) {
		uint16_t cape = tint(cfg, eye_rgb(140, 20, 40), a);
		int16_t base = pair.left.y + pair.left.h + H / 12, tip = pair.left.y + pair.left.h / 4;
		gfx->fillTriangle(W / 30, base, pair.left.x - W / 40, base, W / 30, tip, cape);
		gfx->fillTriangle(W - W / 30, base, pair.right.x + pair.right.w + W / 40, base, W - W / 30, tip, cape);
	}
	pair_body(gfx, cfg, pair, a, 0, 0, 1 + a * 0.05f, 1 - a * 0.5f, eye_rgb(255, 70, 90));
	if (a <= 0) { return true; }
	uint16_t bat = tint(cfg, eye_rgb(70, 40, 90), a);
	int16_t bx = W / 2 + int16_t(cosf(t) * W / 3), by = H / 6 + int16_t(sinf(t * 2) * H / 20), s = W / 24 + 3;
	int16_t flap = (ms / 90) & 1 ? s : -s / 2;
	gfx->fillTriangle(bx, by, bx - s * 2, by - flap, bx - s, by + s / 2, bat);
	gfx->fillTriangle(bx, by, bx + s * 2, by - flap, bx + s, by + s / 2, bat);
	gfx->fillCircle(bx, by, s / 2 + 1, bat);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("vampire", scene);
static_assert(face_preset_valid(preset), "Invalid Vampire face");
} }

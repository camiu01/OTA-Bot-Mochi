/**
 * @file cursor.h
 * @brief A mouse pointer glides in from the corner and clicks the right eye.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace cursor_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The click squeezes the eye and spreads a blue ripple, every three seconds. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 3000;
	float move = t < 1200 ? t / 1200.0f : 1;
	bool click = t >= 1200 && t < 1700;
	int16_t tx = pair.right.x + pair.right.w / 2, ty = pair.right.y + pair.right.h / 2;
	int16_t x = int16_t(W - 8 + (tx - W + 8) * move), y = int16_t(H - 8 + (ty - H + 8) * move);
	for (auto b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, !b.left && click ? 1 - a * 0.8f : 1, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t white = tint(cfg, eye_rgb(250, 250, 250), a), edge = tint(cfg, eye_rgb(40, 40, 50), a);
	int16_t s = W / 11 + 5;
	gfx->fillTriangle(x, y, x, y + s, x + s * 2 / 3, y + s * 2 / 3, white);
	gfx->drawLine(x, y, x, y + s, edge);
	gfx->drawLine(x, y, x + s * 2 / 3, y + s * 2 / 3, edge);
	gfx->drawLine(x + s / 3, y + s * 2 / 3, x + s / 2, y + s + 2, white);
	if (click) { gfx->drawCircle(x, y, int16_t((t - 1200) / 50) + 3, tint(cfg, eye_rgb(120, 200, 255), a)); }
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("cursor", scene);
static_assert(face_preset_valid(preset), "Invalid Cursor face");
} }

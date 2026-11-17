/**
 * @file butterfly.h
 * @brief An orange butterfly flutters in and lands on the right eye, which narrows.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace butterfly_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Flight takes 2.4 seconds; the wings keep flapping after landing. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint32_t t = ms % 6000;
	float land = t < 2400 ? t / 2400.0f : 1, flap = fabsf(sinf(ms * 0.015f));
	int16_t tx = pair.right.x + pair.right.w / 2, ty = pair.right.y - H / 30;
	int16_t x = int16_t(W / 8 + (tx - W / 8) * land + (1 - land) * sinf(ms * 0.006f) * W / 8);
	int16_t y = int16_t(H / 6 + (ty - H / 6) * land);
	for (auto b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, b.left ? 1 : 1 - a * land * 0.3f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t wing = tint(cfg, eye_rgb(255, 150, 60), a), dark = tint(cfg, eye_rgb(60, 40, 30), a);
	int16_t s = W / 22 + 3, w = int16_t(s * (0.3f + 0.7f * flap)) + 1;
	gfx->fillEllipse(x - w, y, w, s, wing);
	gfx->fillEllipse(x + w, y, w, s, wing);
	gfx->drawFastVLine(x, y - s, s * 2, dark);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("butterfly", scene);
static_assert(face_preset_valid(preset), "Invalid Butterfly face");
} }

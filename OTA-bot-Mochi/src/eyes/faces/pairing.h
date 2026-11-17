/**
 * @file pairing.h
 * @brief Eyes take turns dimming while dots travel along a link between two rings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace pairing_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The dimmed eye swaps every 700 ms; dots slide steadily along the link. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	bool flip = (ms / 700) & 1;
	for (auto b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * (b.left == flip ? 0.6f : 0.05f), cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t blue = tint(cfg, eye_rgb(70, 130, 255), a);
	int16_t y = pair.left.y + pair.left.h + H / 12, x0 = pair.left.x + pair.left.w / 2, x1 = pair.right.x + pair.right.w / 2;
	gfx->drawCircle(x0, y, W / 40 + 2, blue);
	gfx->drawCircle(x1, y, W / 40 + 2, blue);
	int16_t step = (x1 - x0) / 6;
	if (step <= 0) { return true; }
	int16_t phase = int16_t((ms / 60) % step);
	for (int i = 0; i < 6; ++i) { gfx->fillCircle(x0 + i * step + phase, y, 1 + W / 160, blue); }
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("pairing", scene);
static_assert(face_preset_valid(preset), "Invalid Pairing face");
} }

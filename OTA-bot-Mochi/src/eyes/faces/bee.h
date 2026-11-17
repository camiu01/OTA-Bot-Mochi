/**
 * @file bee.h
 * @brief Eyes track a striped bee that buzzes along a figure-eight path.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace bee_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The wings alternate every 60 ms; the pair leans toward the bee. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float t = ms * 0.004f;
	int16_t bx = W / 2 + int16_t(sinf(t) * W / 3), by = H / 2 + int16_t(sinf(t * 2) * H / 4);
	pair_body(gfx, cfg, pair, a, int16_t((bx - W / 2) * 0.08f * a), int16_t((by - H / 2) * 0.08f * a), 1, 1 - a * 0.1f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t yellow = tint(cfg, eye_rgb(255, 210, 40), a), black = tint(cfg, eye_rgb(30, 30, 30), a);
	uint16_t wing = tint(cfg, eye_rgb(200, 230, 255), a);
	int16_t r = W / 30 + 3;
	bool flap = (ms / 60) & 1;
	gfx->fillEllipse(bx - 2, by - r, r - 1, flap ? r : r / 2, wing);
	gfx->fillEllipse(bx + 2, by - r, r - 1, flap ? r / 2 : r, wing);
	gfx->fillEllipse(bx, by, r + r / 2, r, yellow);
	gfx->drawFastVLine(bx - r / 3, by - r + 1, r * 2 - 2, black);
	gfx->drawFastVLine(bx + r / 3, by - r + 1, r * 2 - 2, black);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("bee", scene);
static_assert(face_preset_valid(preset), "Invalid Bee face");
} }

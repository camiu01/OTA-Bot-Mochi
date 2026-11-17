/**
 * @file check_engine.h
 * @brief Worried uneven eyes glance up at a blinking amber engine warning lamp.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace check_engine_face {
/** @brief Hollow engine outline built from rectangles: block, intake, top rail, fan and outlet. */
inline void engine(Arduino_GFX *gfx, int16_t cx, int16_t cy, int16_t s, uint16_t ink, uint16_t bg) {
	int16_t t = s / 2 + 1;
	gfx->fillRect(cx - s * 3, cy - s, s * 6, s * 4, ink);
	gfx->fillRect(cx - s * 2, cy - s * 2, s * 3, s, ink);
	gfx->fillRect(cx - s * 3, cy - s * 3, s * 5, t, ink);
	gfx->fillRect(cx - s, cy - s * 3, t, s, ink);
	gfx->fillRect(cx - s * 5, cy, t, s * 2, ink);
	gfx->fillRect(cx - s * 5, cy + s - t / 2, s * 2, t, ink);
	gfx->fillRect(cx + s * 3, cy, s, s * 2, ink);
	gfx->fillRect(cx + s * 4, cy + s / 2, s, s, ink);
	gfx->fillRect(cx - s * 3 + t, cy - s + t, s * 6 - t * 2, s * 4 - t * 2, bg);
}
/** @brief The lamp blinks, then stays lit for the second half of the hold. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.y -= int16_t(a * cfg.screenH / 60);
		face_cinematics::body(gfx, cfg, b, a, 1 - a * (b.left ? 0 : .12f), 1 + a * (b.left ? .1f : -.22f), cfg.fgColor);
	}
	if (a <= 0) { return true; }
	bool lit = (ms % 6000) >= 3600 || (ms % 600) < 380;
	uint16_t amber = eye_mix(eye_rgb(255, 170, 30), cfg.bgColor, uint8_t(a * (lit ? 100 : 22)));
	int16_t s = cfg.screenW / 40 + 1;
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	engine(gfx, cfg.screenW / 2, eye_limit(top - s * 5, s * 3 + 1, cfg.screenH - 1), s, amber, cfg.bgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("check_engine", scene);
static_assert(face_preset_valid(preset), "Invalid Check Engine face");
} }

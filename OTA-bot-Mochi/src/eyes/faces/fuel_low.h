/**
 * @file fuel_low.h
 * @brief Drooping thirsty eyes under a blinking fuel pump and a draining gauge.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace fuel_low_face {
/** @brief Draw the amber dashboard pump from rectangles; the window keeps the panel background. */
inline void pump(Arduino_GFX *gfx, int16_t cx, int16_t cy, int16_t s, uint16_t ink, uint16_t bg) {
	int16_t t = s / 3 + 1;
	gfx->fillRect(cx - s * 2, cy - s * 3, s * 3, s * 6, ink);
	gfx->fillRect(cx - s * 2 + t, cy - s * 3 + t, s * 3 - t * 2, s * 2, bg);
	gfx->fillRect(cx - s * 2 - t, cy + s * 3 - t, s * 3 + t * 2, t, ink);
	gfx->fillRect(cx + s, cy, s + t, t, ink);
	gfx->fillRect(cx + s * 2, cy - s * 2, t, s * 2 + t, ink);
	gfx->fillRect(cx + s * 2 - t, cy - s * 2 - t, t * 2, t, ink);
}
/** @brief The gauge empties across the cycle and turns red near the end; nothing is persisted. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(a * cfg.screenH / 40);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * .42f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t amber = eye_mix(eye_rgb(255, 176, 32), cfg.bgColor, uint8_t(a * 100));
	uint16_t lamp = eye_mix(amber, cfg.bgColor, (ms % 700) < 450 ? 100 : 30);
	int16_t s = cfg.screenW / 48 + 1;
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	pump(gfx, cfg.screenW / 2, eye_limit(top - s * 5, s * 4, cfg.screenH - 1), s, lamp, cfg.bgColor);
	int16_t x = pair.left.x, w = pair.right.x + pair.right.w - x;
	int16_t y = pair.left.y + pair.left.h + cfg.screenH / 12;
	if (w < 6 || y + s + 2 >= cfg.screenH) { return true; }
	float level = .3f - .25f * ((ms % 6000) / 6000.0f);
	uint16_t fill = level < .15f ? eye_mix(eye_rgb(255, 70, 56), cfg.bgColor, uint8_t(a * 100)) : amber;
	gfx->drawRect(x, y, w, s + 2, amber);
	gfx->fillRect(x + 1, y + 1, int16_t((w - 2) * level), s, fill);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("fuel_low", scene);
static_assert(face_preset_valid(preset), "Invalid Fuel Low face");
} }

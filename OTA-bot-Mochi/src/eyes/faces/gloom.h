/**
 * @file gloom.h
 * @brief Drooping half-lidded eyes beneath a small personal raincloud and drizzle.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace gloom_face {
/** @brief A three-lobed grey cloud above the pair drops short blue streaks. */
inline void cloud(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy, uint32_t ms, float a) {
	int16_t r = cfg.screenW / 18 + 2;
	uint16_t grey = eye_mix(eye_rgb(120, 130, 150), cfg.bgColor, uint8_t(a * 100));
	uint16_t rain = eye_mix(eye_rgb(90, 140, 225), cfg.bgColor, uint8_t(a * 100));
	gfx->fillCircle(cx - r, cy, r, grey);
	gfx->fillCircle(cx + r, cy, r, grey);
	gfx->fillCircle(cx, cy - r / 2, r + r / 3, grey);
	int16_t fall = cfg.screenH / 9 + 4, len = cfg.screenH / 40 + 2;
	for (int i = 0; i < 4; ++i) {
		int16_t x = cx - r * 3 / 2 + i * r;
		int16_t y = cy + r + int16_t((ms / 18 + i * fall / 3) % fall);
		gfx->drawFastVLine(x, y, len, rain);
	}
}
/** @brief Sink and narrow the eyes while the cloud drifts gently above them. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(a * cfg.screenH / 30);
		face_cinematics::body(gfx, cfg, b, a, 1 - a * 0.08f, 1 - a * 0.55f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t cx = cfg.screenW / 2 + int16_t(sinf(ms * 0.0015f) * cfg.screenW / 30);
	int16_t cy = eye_limit(top - cfg.screenH / 6, cfg.screenW / 12 + 3, cfg.screenH - 1);
	cloud(gfx, cfg, cx, cy, ms, a);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("gloom", scene);
static_assert(face_preset_valid(preset), "Invalid Gloom face");
} }

/**
 * @file gear_shift.h
 * @brief A knob works through a five-speed gate while the eyes nod at each shift.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace gear_shift_face {
/** @brief Slot centre for gears 0..4 in a three-column gate. */
inline void slot(uint8_t gear, int16_t cx, int16_t cy, int16_t g, int16_t &x, int16_t &y) {
	x = cx + (gear / 2 - 1) * g * 3 / 2; y = cy + ((gear & 1) ? g : -g);
}
/** @brief Draw the gate and move the knob along its legs: to neutral, across, then into gear. */
inline void gate(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cy, int16_t g, uint8_t gear, float t, float a) {
	uint16_t steel = eye_mix(eye_rgb(150, 164, 180), cfg.bgColor, uint8_t(a * 100));
	int16_t cx = cfg.screenW / 2, sp = g * 3 / 2, fx, fy, tx, ty;
	for (int16_t k = -1; k <= 1; ++k) { gfx->fillRect(cx + k * sp, cy - g, 2, (k < 1 ? g * 2 : g) + 1, steel); }
	gfx->fillRect(cx - sp, cy, sp * 2 + 2, 2, steel);
	slot(gear ? gear - 1 : 4, cx, cy, g, fx, fy); slot(gear, cx, cy, g, tx, ty);
	int16_t x = tx, y = ty;
	if (fx == tx) { y = fy + int16_t((ty - fy) * t); }
	else if (t < .34f) { x = fx; y = fy + int16_t((cy - fy) * t * 3); }
	else if (t < .67f) { x = fx + int16_t((tx - fx) * (t - .34f) * 3); y = cy; }
	else { y = cy + int16_t((ty - cy) * (t - .67f) * 3); }
	gfx->fillCircle(x + 1, y, g / 2 + 2, eye_mix(eye_rgb(255, 96, 72), cfg.bgColor, uint8_t(a * 100)));
}
/** @brief One gear every 1.2 seconds; each 300 ms shift squashes the eyes once. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint32_t phase = ms % 6000;
	float t = (phase % 1200) / 300.0f;
	if (t > 1) { t = 1; }
	float kick = t < 1 ? sinf(t * 3.14159265f) : 0;
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(kick * a * cfg.screenH / 50);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * kick * .1f, 1 - a * (.08f + kick * .25f), cfg.fgColor);
	}
	int16_t bottom = pair.left.y + pair.left.h > pair.right.y + pair.right.h ?
		pair.left.y + pair.left.h : pair.right.y + pair.right.h;
	int16_t room = cfg.screenH - bottom;
	if (a > 0 && room >= 14) {
		gate(gfx, cfg, bottom + room / 2, eye_limit(room / 4, 3, cfg.screenW / 12), uint8_t(phase / 1200), t, a);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("gear_shift", scene);
static_assert(face_preset_valid(preset), "Invalid Gear Shift face");
} }

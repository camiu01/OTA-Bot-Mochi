/**
 * @file parking_sensor.h
 * @brief Proximity bars close in from green to flashing red as the eyes tense.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace parking_sensor_face {
/** @brief Stack four distance bars below the eyes; the nearest is the narrowest and reddest. */
inline void bars(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t top, uint8_t level, bool flash, float a) {
	const uint16_t inks[4] = {eye_rgb(80, 220, 110), eye_rgb(190, 220, 70), eye_rgb(255, 160, 40), eye_rgb(255, 60, 50)};
	int16_t bh = cfg.screenH / 36 + 2, gap = bh / 2 + 1;
	for (uint8_t i = 0; i < 4; ++i) {
		int16_t w = int16_t(cfg.screenW * (50 - i * 10) / 100), y = top + (3 - i) * (bh + gap);
		if (w < 2 || y + bh > cfg.screenH) { continue; }
		bool lit = i < level && (flash || i < 3);
		gfx->fillRoundRect((cfg.screenW - w) / 2, y, w, bh, bh / 2,
			eye_mix(inks[i], cfg.bgColor, uint8_t(a * (lit ? 100 : 18))));
	}
}
/** @brief Widen and finally tremble; the level depends only on the cycle timestamp. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	uint32_t phase = ms % 6000;
	uint8_t level = phase < 800 ? 0 : (phase >= 4100 ? 4 : uint8_t(1 + (phase - 800) / 1100));
	float tense = level / 4.0f;
	for (auto b : {pair.left, pair.right}) {
		if (level == 4) { b.x += int16_t((((ms / 60) & 1) ? 1 : -1) * a * (cfg.screenW / 120 + 1)); }
		b.x = eye_limit(b.x, 0, cfg.screenW - b.w);
		face_cinematics::body(gfx, cfg, b, a, 1 + a * .12f * tense, 1 + a * .16f * tense, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	int16_t bottom = pair.left.y + pair.left.h > pair.right.y + pair.right.h ?
		pair.left.y + pair.left.h : pair.right.y + pair.right.h;
	bars(gfx, cfg, bottom + cfg.screenH / 12, level, level < 4 || (ms % 240) < 120, a);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("parking_sensor", scene);
static_assert(face_preset_valid(preset), "Invalid Parking Sensor face");
} }

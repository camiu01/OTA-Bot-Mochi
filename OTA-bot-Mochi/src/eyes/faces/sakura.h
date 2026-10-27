/**
 * @file sakura.h
 * @brief Pink eyes sway beneath a deterministic shower of petals.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace sakura_face {
/** @brief Drift original paired-circle petals without particle storage. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	int16_t r = cfg.screenW / 100 + 1, margin = r * 3;
	uint16_t ink = eye_mix(eye_rgb(255, 165, 205), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 14; ++i) {
		float phase = ((elapsed % 6000 + i * 397) % 6000) / 6000.0f;
		int16_t x = margin + (i * 71 % 101) * (cfg.screenW - margin * 2) / 101;
		x += int16_t(sinf(phase * 6.2831853f + i) * margin);
		int16_t y = margin + int16_t(phase * (cfg.screenH - 2 * margin));
		gfx->fillCircle(x - r / 2, y, r, ink);
		gfx->fillCircle(x + r / 2, y + r, r, ink);
	}
}
/** @brief Sway gently with the petal breeze. */
inline void motion(bool, uint32_t phase, float a, float &, float &, float &dx, float &dy) {
	dx = a * 5 * sinf(phase * 0.0020943951f); dy = a * 3 * cosf(phase * 0.0041887902f);
}
/** @brief Register the petal performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "sakura"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(245, 135, 185); p.scenery = scenery; p.performanceMotion = motion;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sakura face");
} }

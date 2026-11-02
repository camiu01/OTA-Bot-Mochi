/**
 * @file hiccup.h
 * @brief A squash, quick upward hiccup and damped recovery with rising bubbles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace hiccup_face {
/** @brief Repeat two smooth preparation, jolt and recovery beats in the active interval. */
inline void motion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	float t = (phase % 1500) / 1500.0f;
	float center = 0.38f + 0.14f * face_random(phase / 1500, 157u);
	float pulse = expf(-80 * (t - center) * (t - center));
	pulse *= 0.8f + 0.2f * face_random(phase / 1500, 439u);
	sx += a * (0.18f * sinf(t * 6.2831853f) - 0.12f * pulse);
	sy += a * ((left ? 0.35f : 0.15f) * pulse - 0.3f * sinf(t * 6.2831853f));
	dy -= a * 16 * pulse;
	dx += a * (left ? -2 : 2) * pulse;
}

/** @brief Small pastel bubbles escape below the eyes and dissolve before reaching them. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	int16_t bottom = layout.left.y + layout.left.h > layout.right.y + layout.right.h ?
		layout.left.y + layout.left.h : layout.right.y + layout.right.h;
	for (uint8_t i = 0; i < 3; ++i) {
		uint32_t travel = elapsed + i * 430u;
		float t = (travel % 1500) / 1500.0f;
		float drift = face_random(travel / 1500, 173u + i * 47u) * 2 - 1;
		int16_t r = eye_limit(int16_t(cfg.screenW * (0.012f + 0.015f * t)), 1, 12);
		int16_t x = int16_t(cfg.screenW / 2 + sinf(t * 4 + i) * cfg.screenW / 12 + drift * cfg.screenW / 20);
		int16_t y = int16_t(bottom + cfg.screenH / 12 + (1 - t) * cfg.screenH / 10);
		y = eye_limit(y, r, int16_t(cfg.screenH - r - 1));
		uint16_t ink = eye_mix(eye_rgb(151, 222, 230), cfg.bgColor, uint8_t(a * sinf(t * 3.14159265f) * 80));
		gfx->drawCircle(x, y, r, ink);
	}
}

/** @brief Keep this physical gag separate from laughing and sneezing. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "hiccup"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(201, 232, 242);
	p.performanceMotion = motion; p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid hiccup face");
} }

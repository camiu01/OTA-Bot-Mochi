/**
 * @file drift.h
 * @brief Counter-steering eyes slide sideways over tyre smoke and skid marks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace drift_face {
/** @brief Three bounded puffs and one skid stroke per eye trail opposite to the current slide. */
inline void smoke(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t ms, float a, int16_t side) {
	for (uint8_t i = 0; i < 3; ++i) {
		float t = float((ms + i * 260 + (b.left ? 0 : 130)) % 780) / 780;
		int16_t r = int16_t(cfg.screenW / 60 + t * cfg.screenW / 28) + 1;
		int16_t x = b.x + b.w / 2 - int16_t(side * t * cfg.screenW / 7);
		int16_t y = b.y + b.h + r - int16_t(t * cfg.screenH / 30);
		gfx->fillCircle(x, y, r, eye_mix(eye_rgb(188, 194, 204), cfg.bgColor, uint8_t(a * (1 - t) * 100)));
	}
	uint16_t rubber = eye_mix(eye_rgb(92, 96, 104), cfg.bgColor, uint8_t(a * 100));
	int16_t y = eye_limit(b.y + b.h + cfg.screenH / 8, 0, cfg.screenH - 3);
	for (int16_t t = 0; t < 2; ++t) {
		gfx->drawLine(b.x, y + t, b.x + b.w, y + t + side * cfg.screenH / 40, rubber);
	}
}
/** @brief Opposite vertical offsets suggest body roll without rotating the configured style. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float slide = sinf((ms % 6000) * .0020943951f);
	for (auto b : {pair.left, pair.right}) {
		b.x = eye_limit(b.x + int16_t(slide * a * cfg.screenW / 14), 0, cfg.screenW - b.w);
		b.y += int16_t((b.left ? -1 : 1) * slide * a * cfg.screenH / 22);
		if (a > 0) { smoke(gfx, cfg, b, ms, a, slide >= 0 ? 1 : -1); }
		face_cinematics::body(gfx, cfg, b, a, 1 + a * .08f, 1 - a * .3f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("drift", scene);
static_assert(face_preset_valid(preset), "Invalid Drift face");
} }

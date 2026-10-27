/**
 * @file stargazing.h
 * @brief stargazing definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace stargazing_face {
using namespace face_primitives;
inline void local_eye_performance_stars(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);
inline float local_eye_stargazing_look(uint32_t elapsed);


/** @brief stargazing local_eye_performance_stars implementation. */
inline void local_eye_performance_stars(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	int16_t margin = cfg.screenW / 16 + 2;
	for (uint8_t i = 0; i < 28; ++i) {
		int16_t x = margin + ((i * 73 + 17) % 211) * (cfg.screenW - 2 * margin) / 211;
		int16_t y = margin + ((i * i * 11 + 7) % 101) * (cfg.screenH * 3 / 4 - margin) / 101;
		float light = 0.55f + 0.35f * sinf((elapsed % 6000) * 0.00314159265f + i * 1.7f);
		uint16_t tint = i % 3 ? eye_rgb(180, 210, 255) : eye_rgb(255, 220, 160);
		uint16_t ink = eye_mix(tint, cfg.bgColor, uint8_t(a * light * 100));
		int16_t r = i % 7 == 0 ? cfg.screenW / 120 + 1 : 0;
		gfx->fillRect(x - r, y, r * 2 + 1, 1, ink);
		gfx->fillRect(x, y - r, 1, r * 2 + 1, ink);
	}
}

/** @brief stargazing local_eye_stargazing_look implementation. */
inline float local_eye_stargazing_look(uint32_t elapsed) {
	uint32_t p = elapsed % 6000;
	float t = p < 1500 ? 0 : (p < 2100 ? (p - 1500) / 600.0f :
		(p < 3900 ? 1 : (p < 4500 ? (4500 - p) / 600.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief stargazing scenery procedure. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	(void)layout;
	local_eye_performance_stars(gfx, cfg, elapsed, a * local_eye_stargazing_look(elapsed));
}

/** @brief stargazing layout procedure. */
inline void layout(const EyeConfig &cfg, EyeBox &box, uint32_t elapsed, float a) {
	EyeBox *b = &box;

			float look = local_eye_stargazing_look(elapsed);
			int16_t w = eye_limit(b->w * 35 / 100, 6, cfg.screenW);
			int16_t h = eye_limit(int16_t(b->h * (0.35f + 0.035f * look)), 4, cfg.screenH);
			int16_t cx = cfg.screenW / 2 + (b->left ? -1 : 1) * (w / 2 + cfg.screenW / 40);
			int16_t y = cfg.screenH - cfg.screenH / 20 - h - int16_t(look * cfg.screenH / 40);
			b->x += int16_t((cx - w / 2 - b->x) * a);
			b->y += int16_t((y - b->y) * a);
			b->w += int16_t((w - b->w) * a); b->h += int16_t((h - b->h) * a);
			b->radius = eye_limit(b->radius, 0, (b->w < b->h ? b->w : b->h) / 2);
			return;

}

/** @brief stargazing drawing procedure. */
inline void drawing(EyeState &st, const EyeLayout &layout, uint32_t elapsed) {
	st.saccadeY = -int16_t(layout.left.h * local_eye_stargazing_look(elapsed) / 2);
}

/** @brief Immutable stargazing face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "stargazing";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(150, 190, 245);
	p.scenery = scenery;
	p.layout = layout;
	p.drawing = drawing;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid stargazing face parameters");
} }

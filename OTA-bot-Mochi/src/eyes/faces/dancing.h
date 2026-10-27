/**
 * @file dancing.h
 * @brief dancing definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace dancing_face {
using namespace face_primitives;
inline EyeBox local_eye_equalizer_bar(const EyeConfig &cfg, const EyeLayout &layout, uint8_t index, uint32_t elapsed);
inline uint16_t local_eye_equalizer_ink(uint8_t index);
inline void local_eye_performance_equalizer(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed, float a);


/** @brief dancing local_eye_equalizer_bar implementation. */
inline EyeBox local_eye_equalizer_bar(const EyeConfig &cfg, const EyeLayout &layout, uint8_t index, uint32_t elapsed) {
	int16_t span = layout.right.x + layout.right.w - layout.left.x, stride = span / 12;
	int16_t w = stride * 2 / 3;
	float beat = 0.25f + 0.75f * fabsf(sinf((elapsed % 6000) * 0.0062831853f + index * 0.7f));
	int16_t h = int16_t(layout.left.h * beat);
	return {int16_t(layout.left.x + index * stride), int16_t(cfg.screenH / 2 - h / 2),
		w, h, int16_t((w < h ? w : h) / 2), index < 6};
}

/** @brief dancing local_eye_equalizer_ink implementation. */
inline uint16_t local_eye_equalizer_ink(uint8_t index) {
	static constexpr uint16_t colors[] = {eye_rgb(100, 225, 200), eye_rgb(100, 190, 245),
		eye_rgb(175, 130, 245), eye_rgb(245, 135, 190)};
	return colors[index / 3];
}

/** @brief dancing local_eye_performance_equalizer implementation. */
inline void local_eye_performance_equalizer(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed, float a) {
	for (uint8_t i = 0; i < 12; ++i) {
		if (i == 2 || i == 9) { continue; }
		EyeBox bar = local_eye_equalizer_bar(cfg, layout, i, elapsed);
		gfx->fillRoundRect(bar.x, bar.y, bar.w, bar.h, bar.radius,
			eye_mix(local_eye_equalizer_ink(i), cfg.bgColor, uint8_t(a * a * 100)));
	}
	// Paint morphing eyes last: background columns must never cut slots into them.
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &eye : eyes) {
		uint8_t index = eye.left ? 2 : 9;
		EyeConfig body = cfg;
		body.fgColor = eye_mix(local_eye_equalizer_ink(index), cfg.fgColor, uint8_t(a * 100));
		eye_scene_body(gfx, body, eye_scene_box(eye, local_eye_equalizer_bar(cfg, layout, index, elapsed), a), a);
	}
}

/** @brief dancing performanceScene procedure. */
inline bool performanceScene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	local_eye_performance_equalizer(gfx, cfg, layout, elapsed, a); return true;
}

/** @brief Immutable dancing face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "dancing";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(120, 220, 255);
	p.performanceScene = performanceScene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid dancing face parameters");
} }

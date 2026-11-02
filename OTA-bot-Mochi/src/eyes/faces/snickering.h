/**
 * @file snickering.h
 * @brief Uneven laughing chevrons bounce beneath descending comic streaks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace snickering_face {
/** @brief Morph occupied row spans into positive chevron strokes, never carving a full eye. */
inline void eye(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, float a) {
	int16_t thick = eye_limit(int16_t(b.w / 8), 2, int16_t(b.w / 3));
	for (int16_t row = 0; row < b.h; ++row) {
		int16_t left, right;
		eye_body_span(cfg, b, row, left, right);
		float tip = 1 - fabsf(2.0f * row / (b.h - 1) - 1);
		int16_t start = int16_t(tip * (b.w - thick));
		if (!b.left) { start = int16_t(b.w - thick - start); }
		left = int16_t(lroundf(left + (start - left) * a));
		right = int16_t(lroundf(right + (start + thick - 1 - right) * a));
		if (right >= left) { gfx->fillRect(b.x + left, b.y + row, right - left + 1, 1, cfg.fgColor); }
	}
	if (a < 0.3f) {
		EyeConfig pupils = cfg;
		pupils.pupilColor = eye_mix(cfg.fgColor, cfg.pupilColor, uint8_t(a * 333));
		pupils.highlightColor = eye_mix(cfg.fgColor, cfg.highlightColor, uint8_t(a * 333));
		eye_pupils(gfx, pupils, EyeState{}, b);
	}
}

/** @brief Morph the real style into asymmetric, rhythmically bouncing closed eyes. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	float bounce = sinf((elapsed % 6000) * 0.018849556f);
	for (const EyeBox &source : {layout.left, layout.right}) {
		EyeBox target = source;
		target.h = int16_t(cfg.screenH / 7);
		target.radius = int16_t((target.w < target.h ? target.w : target.h) / 3);
		target.y = int16_t(cfg.screenH / 2 - target.h / 2 + (source.left ? -4 : 4) * eye_scale(cfg) / 100
			+ bounce * cfg.screenH / 55);
		EyeBox b = eye_scene_box(source, target, a);
		EyeConfig ink = cfg;
		ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		eye(gfx, ink, b, a);
	}
	uint16_t streak = eye_mix(eye_rgb(94, 144, 194), cfg.bgColor, uint8_t(a * 42));
	for (uint8_t i = 0; i < 9; ++i) {
		int16_t x = int16_t(cfg.screenW * (i + 1) / 10);
		float t = ((elapsed % 6000 + i * 127) % 900) / 900.0f;
		int16_t y = int16_t(cfg.screenH * (0.12f + 0.16f * t));
		gfx->drawLine(x, y, x + cfg.screenW / 65, y + cfg.screenH / 28, streak);
	}
	return true;
}

/** @brief Register a distinct comic laugh, without a mouth or copied artwork. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "snickering"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(184, 222, 251); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid snickering face");
} }

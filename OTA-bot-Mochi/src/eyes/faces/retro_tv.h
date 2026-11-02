/**
 * @file retro_tv.h
 * @brief The original eye pair becomes a little television with moving color bars.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace retro_tv_face {
/** @brief Draw an original CRT casing and bring the configured eyes inside its screen. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	int16_t w = cfg.screenW * 4 / 5, h = cfg.screenH * 2 / 5;
	int16_t x = (cfg.screenW - w) / 2, y = (cfg.screenH - h) / 2;
	int16_t pad = cfg.screenW / 35, controls = w / 8;
	uint16_t shell = eye_mix(eye_rgb(135, 155, 177), cfg.bgColor, uint8_t(a * 100));
	uint16_t glass = eye_mix(eye_rgb(24, 37, 48), cfg.bgColor, uint8_t(a * 100));
	gfx->drawLine(cfg.screenW / 2, y, cfg.screenW / 3, y - h / 4, shell);
	gfx->drawLine(cfg.screenW / 2, y, cfg.screenW * 2 / 3, y - h / 5, shell);
	gfx->fillRoundRect(x, y, w, h, pad * 2, shell);
	gfx->fillRoundRect(x + pad, y + pad, w - controls - pad * 3, h - pad * 2, pad, glass);
	for (uint8_t i = 0; i < 2; ++i) {
		gfx->fillCircle(x + w - controls / 2 - pad, y + h * (i + 1) / 3, pad, glass);
	}
	const uint16_t bars[] = {eye_rgb(219, 168, 106), eye_rgb(118, 192, 184), eye_rgb(183, 132, 191)};
	int16_t scan = int16_t(y + pad + ((elapsed % 6000) % 1800) * (h - pad * 3) / 1800);
	for (uint8_t i = 0; i < 3; ++i) {
		uint16_t ink = eye_mix(bars[i], glass, uint8_t(a * 35));
		gfx->fillRect(x + pad * 2 + i * (w - controls - pad * 5) / 3, scan,
			(w - controls - pad * 5) / 3, pad, ink);
	}
	for (const EyeBox &source : {layout.left, layout.right}) {
		int16_t ew = w / 6, eh = h / 3;
		int16_t cx = int16_t(x + (w - controls) * (source.left ? 0.32f : 0.68f));
		EyeBox target{int16_t(cx - ew / 2), int16_t(y + h / 2 - eh / 2), ew, eh, int16_t(ew / 3), source.left};
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, source.left, elapsed);
		eye_scene_body(gfx, ink, eye_scene_box(source, target, a), a);
	}
	return true;
}

/** @brief Register the TV scene with no source video, text, or logos. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "retro_tv"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(195, 235, 222); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid retro TV face");
} }

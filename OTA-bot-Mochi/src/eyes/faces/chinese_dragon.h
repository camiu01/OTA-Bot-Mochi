/**
 * @file chinese_dragon.h
 * @brief Original red Chinese dragon with branched gold horns, whiskers and a coiling body.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace chinese_dragon_face {
/** @brief Curl a tapered red body below the head, with sparse gold dorsal fins. */
inline void body(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	uint16_t red = eye_mix(eye_rgb(205, 43, 35), cfg.bgColor, uint8_t(a * 100));
	uint16_t gold = eye_mix(eye_rgb(244, 193, 89), cfg.bgColor, uint8_t(a * 100));
	float wave = (elapsed % 6000) * 0.0020943951f;
	for (uint8_t step = 0; step <= 64; ++step) {
		float t = step / 64.0f;
		int16_t r = int16_t(cfg.screenW * (0.008f + t * 0.047f));
		int16_t x = int16_t(cfg.screenW * (0.52f + sinf(t * 7.5f) * 0.22f * (1 - t * 0.35f))
			+ sinf(t * 9 - wave) * cfg.screenW * 0.012f);
		int16_t y = int16_t(cfg.screenH * (0.91f - t * 0.39f));
		if (step % 6 == 0) {
			gfx->fillTriangle(x - r, y, x - r * 2, y + r, x, y + r, gold);
		}
		gfx->fillCircle(x, y, r, red);
		gfx->fillCircle(x + r / 3, y, r / 3, gold);
	}
}

/** @brief Pair antler-like horns and long animated whiskers, not western wings. */
inline void ornaments(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cy, uint32_t elapsed, float a) {
	int16_t w = cfg.screenW, h = cfg.screenH, cx = w / 2;
	uint16_t gold = eye_mix(eye_rgb(244, 193, 89), cfg.bgColor, uint8_t(a * 100));
	for (int16_t d : {-1, 1}) {
		int16_t x = cx + d * w / 7, y = cy - h / 10;
		gfx->fillTriangle(x - w / 50, y, x + w / 50, y, x + d * w / 18, y - h / 7, gold);
		gfx->fillTriangle(x + d * w / 30, y - h / 14, x + d * w / 7, y - h / 10,
			x + d * w / 20, y - h / 20, gold);
		int16_t px = cx + d * w / 9, py = cy + h / 13;
		for (uint8_t step = 1; step <= 20; ++step) {
			float t = step / 20.0f;
			int16_t nx = int16_t(cx + d * w * (0.11f + t * 0.31f));
			int16_t ny = int16_t(cy + h * (0.077f - sinf(t * 3.14159265f) * 0.08f)
				+ sinf((elapsed % 6000) * 0.0020943951f + t * 5) * t * h / 100);
			gfx->drawLine(px, py, nx, ny, gold);
			gfx->drawLine(px, py + 1, nx, ny + 1, gold);
			px = nx; py = ny;
		}
	}
}

/** @brief Assemble a broad red head, pointed ears and a warm gold muzzle. */
inline void head(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cy, float a) {
	int16_t w = cfg.screenW, h = cfg.screenH, cx = w / 2;
	uint16_t mane = eye_mix(eye_rgb(157, 34, 28), cfg.bgColor, uint8_t(a * 100));
	uint16_t red = eye_mix(eye_rgb(226, 51, 43), cfg.bgColor, uint8_t(a * 100));
	uint16_t gold = eye_mix(eye_rgb(244, 193, 89), cfg.bgColor, uint8_t(a * 100));
	for (int16_t d : {-1, 1}) {
		gfx->fillTriangle(cx + d * w / 6, cy - h / 15, cx + d * w / 3, cy - h / 9,
			cx + d * w / 4, cy + h / 14, red);
		gfx->fillTriangle(cx + d * w / 5, cy, cx + d * w / 3, cy + h / 12,
			cx + d * w / 7, cy + h / 8, mane);
	}
	gfx->fillRoundRect(cx - w / 4, cy - h / 11, w / 2, h / 5, w / 13, red);
	gfx->fillTriangle(cx - w / 20, cy - h / 11, cx, cy - h / 7, cx + w / 20, cy - h / 11, gold);
	gfx->fillRoundRect(cx - w / 8, cy + h / 23, w / 4, h / 14, h / 40, gold);
	int16_t r = w / 100;
	gfx->fillCircle(cx - w / 24, cy + h / 14, r, mane);
	gfx->fillCircle(cx + w / 24, cy + h / 14, r, mane);
}

/** @brief Keep entry/exit reversible while the selected eyes become the dragon's gold gaze. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	int16_t cy = int16_t(cfg.screenH * 0.39f + sinf((elapsed % 6000) * 0.0020943951f) * cfg.screenH / 130);
	if (a > 0) {
		body(gfx, cfg, elapsed, a);
		head(gfx, cfg, cy, a);
		ornaments(gfx, cfg, cy, elapsed, a);
	}
	for (const EyeBox &source : {layout.left, layout.right}) {
		int16_t w = cfg.screenW / 9, h = cfg.screenH / 23;
		int16_t cx = int16_t(cfg.screenW * (source.left ? 0.39f : 0.61f));
		EyeBox target{int16_t(cx - w / 2), int16_t(cy - h / 2), w, h, int16_t(h / 3), source.left};
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, source.left, elapsed);
		eye_scene_body(gfx, ink, eye_scene_box(source, target, a), a);
	}
	return true;
}

/** @brief Register an original six-second red-and-gold Chinese dragon performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "chinese_dragon"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(255, 221, 134); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid Chinese dragon face");
} }

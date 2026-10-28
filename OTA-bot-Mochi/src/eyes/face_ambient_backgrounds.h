/**
 * @file face_ambient_backgrounds.h
 * @brief Low-contrast themed accents confined to the space above and below eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_backgrounds.h"

namespace face_backgrounds {

/** @brief A quiet cherry branch with four petals drifting a few pixels during the scene. */
inline void petals(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, pair, cfg.screenH / 20);
	if (top < 12) { return; }
	uint16_t branch = entry_mix(st, eye_rgb(146, 116, 136), cfg.bgColor, 40);
	uint16_t bloom = entry_mix(st, eye_rgb(225, 162, 185), cfg.bgColor, 52);
	int16_t y = top / 3;
	gfx->drawLine(0, y, cfg.screenW / 3, top / 2, branch);
	uint32_t phase = st.lastMs % 6000;
	float sway = phase > 1500 && phase < 4500 ?
		sinf((phase - 1500) * 0.0010471976f) : 0;
	for (uint8_t i = 0; i < 4; ++i) {
		int16_t x = int16_t(cfg.screenW * (i + 1) / 6);
		int16_t py = int16_t(top * (i % 2 + 1) / 4);
		int16_t drift = int16_t(sway * sway * cfg.screenW / 80);
		int16_t r = eye_limit(int16_t(cfg.screenW / 150), 1, 3);
		gfx->fillCircle(x + drift, py, r, bloom);
		gfx->fillCircle(x + drift + r, py + r, r, bloom);
	}
}

/** @brief Distant, stationary bamboo silhouettes leave the eye row entirely clear. */
inline void grove(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t bottom = pair_bottom(cfg, pair, cfg.screenH / 20);
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h < 10) { return; }
	uint16_t stem = entry_mix(st, eye_rgb(100, 154, 116), cfg.bgColor, 44);
	uint16_t leaf = entry_mix(st, eye_rgb(144, 177, 126), cfg.bgColor, 38);
	for (uint8_t i = 0; i < 4; ++i) {
		int16_t x = int16_t(cfg.screenW * (i * 2 + 1) / 8);
		int16_t y = int16_t(bottom + h * (i % 2 + 1) / 5);
		int16_t w = eye_limit(int16_t(cfg.screenW / 100), 1, 4);
		gfx->fillRect(x, y, w, int16_t(cfg.screenH - y), stem);
		int16_t node = int16_t(y + h / 3);
		gfx->drawLine(x, node, x + cfg.screenW / 25, node - h / 8, leaf);
		gfx->drawLine(x, node + h / 4, x - cfg.screenW / 30, node + h / 8, leaf);
	}
}

/** @brief A sparse skyline below the lamps, without competing bright lane markings. */
inline void city(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t bottom = pair_bottom(cfg, pair, cfg.screenH / 20);
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h < 10) { return; }
	uint16_t building = entry_mix(st, eye_rgb(100, 119, 142), cfg.bgColor, 38);
	uint16_t window = entry_mix(st, eye_rgb(186, 166, 119), cfg.bgColor, 42);
	for (uint8_t i = 0; i < 5; ++i) {
		int16_t x = int16_t(cfg.screenW * (i * 2 + 1) / 12);
		int16_t w = int16_t(cfg.screenW / 12);
		int16_t y = int16_t(bottom + h * (i % 3 + 2) / 6);
		gfx->fillRect(x, y, w, int16_t(cfg.screenH - y), building);
		gfx->fillRect(x + w / 2, y + h / 12, 1, 2, window);
	}
}

/** @brief A sparse, stationary night sky above the crescent eyes. */
inline void stars(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, pair, cfg.screenH / 20);
	if (top < 10) { return; }
	uint16_t dim = entry_mix(st, eye_rgb(148, 171, 210), cfg.bgColor, 38);
	uint16_t light = entry_mix(st, eye_rgb(194, 204, 226), cfg.bgColor, 48);
	for (uint8_t i = 0; i < 7; ++i) {
		int16_t x = int16_t(cfg.screenW * (i * 2 + 2) / 17);
		int16_t y = int16_t(top * (i % 3 + 1) / 5);
		gfx->fillRect(x, y, 1, 1, dim);
		if (i == 2 || i == 5) {
			gfx->drawLine(x - 1, y, x + 1, y, light);
			gfx->drawLine(x, y - 1, x, y + 1, light);
		}
	}
}

/** @brief Two thin, still water ripples below the relaxed eye pair. */
inline void ripples(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t bottom = pair_bottom(cfg, pair, cfg.screenH / 20);
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h < 10) { return; }
	uint16_t ink = entry_mix(st, eye_rgb(116, 164, 166), cfg.bgColor, 44);
	for (uint8_t row = 0; row < 2; ++row) {
		int16_t y = int16_t(bottom + h * (row + 1) / 3);
		int16_t begin = int16_t(cfg.screenW / (row ? 6 : 10));
		int16_t end = int16_t(cfg.screenW - begin);
		int16_t previous = y;
		for (int16_t x = begin; x < end; ++x) {
			float phase = float(x - begin) / (end - begin) * 6.2831853f;
			int16_t next = int16_t(y + sinf(phase + row) * h / 24);
			if (x > begin) { gfx->drawLine(x - 1, previous, x, next, ink); }
			previous = next;
		}
	}
}

/** @brief Two small cloud silhouettes above the eyes, drifting only a few pixels. */
inline void clouds(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, pair, cfg.screenH / 20);
	if (top < 12) { return; }
	uint16_t ink = entry_mix(st, eye_rgb(164, 164, 192), cfg.bgColor, 42);
	uint32_t phase = st.lastMs % 6000;
	float sway = phase > 1500 && phase < 4500 ?
		sinf((phase - 1500) * 0.0010471976f) : 0;
	for (uint8_t i = 0; i < 2; ++i) {
		int16_t r = eye_limit(int16_t(top / 9), 2, int16_t(cfg.screenW / 24));
		int16_t x = int16_t(cfg.screenW * (i ? 3 : 1) / 4);
		x += int16_t(sway * sway * cfg.screenW / 100) * (i ? -1 : 1);
		int16_t y = int16_t(top * (i + 2) / 5);
		gfx->fillCircle(x - r, y, r, ink);
		gfx->fillCircle(x, y - r / 2, r + 1, ink);
		gfx->fillCircle(x + r, y, r, ink);
		gfx->fillRect(x - r, y, r * 2, r, ink);
	}
}

}  // namespace face_backgrounds

/**
 * @file eye_effects.h
 * @brief Decorative gauge, cycling glow and headlight cones with explicit dirty bounds.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_shapes.h"
#include "eye_palette.h"
#include <math.h>

/** @brief Blend RGB565 channels without allocating a framebuffer. @param ink Foreground. @param bg Background. @param amount Percent foreground. @return Blended pixel. */
inline uint16_t eye_mix(uint16_t ink, uint16_t bg, uint8_t amount) {
	uint16_t r = (((ink >> 11) * amount + (bg >> 11) * (100 - amount)) / 100);
	uint16_t g = ((((ink >> 5) & 63) * amount + ((bg >> 5) & 63) * (100 - amount)) / 100);
	uint16_t b = (((ink & 31) * amount + (bg & 31) * (100 - amount)) / 100);
	return uint16_t((r << 11) | (g << 5) | b);
}

/** @brief Cycle from the selected eye color through six hues and back, without editing preferences. @param now Elapsed milliseconds. @param base Selected eye ink. @return RGB565 body ink. */
inline uint16_t eye_glow_color(uint32_t now, uint16_t base) {
	static constexpr uint16_t colors[] = {eye_rgb(255, 112, 190), eye_rgb(160, 128, 255),
		eye_rgb(100, 210, 255), eye_rgb(100, 255, 185), eye_rgb(255, 215, 112), eye_rgb(255, 148, 128)};
	uint32_t phase = now % 9600;
	if (phase < 1200) { return base; }
	phase -= 1200;
	uint8_t index = uint8_t(phase / 1200);
	uint16_t from = index == 0 ? base : colors[index - 1];
	uint16_t to = index == 6 ? base : colors[index];
	return eye_mix(to, from, uint8_t((phase % 1200) / 12));
}

/** @brief Expanded halo clipped to the actual canvas. @param cfg Config. @param b Eye. @param pad Expansion. @return Bounded halo. */
inline EyeBox eye_halo_box(const EyeConfig &cfg, const EyeBox &b, int16_t pad) {
	int16_t x = eye_limit(b.x - pad, 0, cfg.screenW - 1);
	int16_t y = eye_limit(b.y - pad, 0, cfg.screenH - 1);
	return {x, y, int16_t(eye_limit(b.x + b.w + pad, x + 1, cfg.screenW) - x),
		int16_t(eye_limit(b.y + b.h + pad, y + 1, cfg.screenH) - y), int16_t(b.radius + pad), b.left};
}

/** @brief Gauge dial rectangle, sized independently of the current eye style. @param cfg Config. @return Bounded square. */
inline EyeBox eye_gauge_box(const EyeConfig &cfg) {
	int16_t size = (cfg.screenW < cfg.screenH ? cfg.screenW : cfg.screenH) * 3 / 4;
	return {int16_t((cfg.screenW - size) / 2), int16_t((cfg.screenH - size) / 2), size, size, 0, true};
}

/** @brief Include halos/cones/dial in the normal dirty-region union. @param cfg Config. @param layout Eyes. @return Actual effect bounds. */
inline EyeLayout eye_effect_bounds(const EyeConfig &cfg, EyeLayout layout) {
	if (cfg.expression == GAUGE) {
		EyeBox b = eye_gauge_box(cfg);
		int16_t x = eye_limit(b.x < layout.left.x ? b.x : layout.left.x, 0, cfg.screenW - 1);
		int16_t y = b.y < layout.left.y ? b.y : layout.left.y;
		if (layout.right.y < y) { y = layout.right.y; }
		int16_t right = b.x + b.w > layout.right.x + layout.right.w ? b.x + b.w : layout.right.x + layout.right.w;
		int16_t bottom = b.y + b.h;
		if (layout.left.y + layout.left.h > bottom) { bottom = layout.left.y + layout.left.h; }
		if (layout.right.y + layout.right.h > bottom) { bottom = layout.right.y + layout.right.h; }
		EyeBox bounds = {x, y, int16_t(right - x), int16_t(bottom - y), 0, true};
		return {bounds, bounds};
	}
	if (cfg.expression == GLOW) {
		int16_t pad = 6 * eye_scale(cfg) / 100;
		return {eye_halo_box(cfg, layout.left, pad), eye_halo_box(cfg, layout.right, pad)};
	}
	if (cfg.expression == HEADLIGHTS) {
		EyeBox *boxes[] = {&layout.left, &layout.right};
		for (EyeBox *b : boxes) {
			int16_t half = b->h * 3 / 4 + 20 * eye_scale(cfg) / 100 + 3;
			int16_t cy = b->y + b->h / 2;
			int16_t top = eye_limit(cy - half, 0, cfg.screenH - 1);
			int16_t bottom = eye_limit(cy + half + 1, top + 1, cfg.screenH);
			b->x = 0; b->w = cfg.screenW; b->y = top; b->h = bottom - top;
		}
	}
	return layout;
}

/** @brief Grow a joined cone outward over 1.2 seconds, then pulse softly. @param gfx Canvas. @param cfg Config. @param b Eye. @param now Time since mood entry. */
inline void eye_headlight(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t now) {
	int16_t reach = b.left ? b.x : cfg.screenW - b.x - b.w;
	if (reach <= 0) { return; }
	float opening = now >= 1200 ? 1 : now / 1200.0f;
	opening = opening * opening * (3 - 2 * opening);
	int16_t tip = int16_t(reach * opening);
	if (tip == 0) { return; }
	float wave = sinf((now % 2400) * 0.0026179939f + (b.left ? 0.0f : 1.5707963f));
	int16_t spread = b.h / 4 + 16 * eye_scale(cfg) / 100;
	int16_t cy = b.y + b.h / 2;
	const uint16_t soft = eye_mix(eye_rgb(255, 255, 255), cfg.fgColor, 30);
	// Extend beneath the eye so rounded corners meet the beam without a black seam.
	for (int16_t d = -b.w / 2; d <= tip; ++d) {
		int16_t half = b.h / 2 + int32_t(d) * spread / reach;
		if (half < 0) { continue; }
		int16_t x = b.left ? b.x - d : b.x + b.w - 1 + d;
		int32_t fade = d < 0 ? 100 : 100 - int32_t(d) * 100 / reach;
		fade = 25 + fade * 3 / 4;
		fade = int32_t(fade * opening);
		if (opening < 1 && d > tip - 6 && d > 0) { fade = fade * (tip - d) / 6; }
		for (int16_t layer = 0; layer < 8; ++layer) {
			int16_t inner = half - half * layer / 32;
			uint8_t amount = uint8_t((20 + layer * 8 + 6 * wave) * fade / 100);
			int16_t top = eye_limit(cy - inner, 0, cfg.screenH - 1);
			int16_t bottom = eye_limit(cy + inner + 1, top + 1, cfg.screenH);
			gfx->fillRect(x, top, 1, bottom - top, eye_mix(soft, cfg.bgColor, amount));
		}
	}
}

/** @brief Paint effects behind normal style bodies; palette changes are frame-local. @param gfx Canvas. @param cfg Local config. @param st State. @param layout Eyes. */
inline void eye_effect_background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	if (cfg.expression == HEADLIGHTS) {
		eye_headlight(gfx, cfg, layout.left, st.lastMs);
		eye_headlight(gfx, cfg, layout.right, st.lastMs);
	}
	if (cfg.expression != GLOW) { return; }
	cfg.fgColor = eye_glow_color(st.lastMs, cfg.fgColor);
	for (int16_t pad = 6; pad >= 2; pad -= 2) {
		EyeConfig halo = cfg;
		halo.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(48 - pad * 6));
		eye_body(gfx, halo, eye_halo_box(cfg, layout.left, pad * eye_scale(cfg) / 100));
		eye_body(gfx, halo, eye_halo_box(cfg, layout.right, pad * eye_scale(cfg) / 100));
	}
}

/** @brief Accelerate, bounce against a decorative limiter, then ease back to idle. @param elapsed Mood-relative milliseconds. @return Needle fraction, 0..1. */
inline float eye_gauge_sweep(uint32_t elapsed) {
	uint32_t phase = elapsed % 12000;
	if (phase < 2000 || phase >= 10000) { return 0; }
	phase = (phase - 2000) % 4000;
	if (phase < 1600) {
		float t = phase / 1600.0f;
		return t * t * (3 - 2 * t);
	}
	if (phase < 3200) {
		uint32_t beat = (phase - 1600) % 400;
		float cut = beat < 80 ? beat / 80.0f : (400 - beat) / 320.0f;
		return 1 - 0.18f * cut * cut * (3 - 2 * cut);
	}
	float t = (phase - 3200) / 800.0f;
	return 1 - t * t * (3 - 2 * t);
}

/** @brief Draw an autonomous 270-degree dial with redline and limiter bounce, never actual speed. @param gfx Canvas. @param cfg Config. @param now Timestamp. */
inline void eye_gauge_draw(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t now, uint8_t opacity = 100) {
	EyeBox b = eye_gauge_box(cfg);
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, r = b.w / 2 - 3;
	const uint16_t redline = eye_mix(eye_rgb(238, 65, 54), cfg.bgColor, opacity);
	for (uint8_t i = 0; i <= 54; ++i) {
		float angle = 2.3561945f + i * 0.08726646f;
		int16_t outer = r, inner = r - (i % 6 ? 3 : 9);
		uint16_t ink = i >= 42 ? redline : cfg.fgColor;
		gfx->drawLine(int16_t(cx + cosf(angle) * inner), int16_t(cy + sinf(angle) * inner),
			int16_t(cx + cosf(angle) * outer), int16_t(cy + sinf(angle) * outer), ink);
	}
	float sweep = eye_gauge_sweep(now);
	float angle = 2.3561945f + sweep * 4.712389f;
	gfx->drawLine(cx, cy, int16_t(cx + cosf(angle) * (r - 14)), int16_t(cy + sinf(angle) * (r - 14)), redline);
	gfx->fillCircle(cx, cy, 3, cfg.fgColor);
}

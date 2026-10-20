/**
 * @file eye_performance_draw.h
 * @brief Original procedural scenery; no GIF decoding, borrowed pixels, or extra framebuffers.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_performances.h"
#include "eye_scenes.h"
#include "eye_performance_objects.h"

/** @brief Resolve one actual equalizer column, including the two eye morph targets. @param cfg Config. @param layout Eyes. @param index Column. @param elapsed Entry time. @return Animated column. */
inline EyeBox eye_equalizer_bar(const EyeConfig &cfg, const EyeLayout &layout, uint8_t index, uint32_t elapsed) {
	int16_t span = layout.right.x + layout.right.w - layout.left.x, stride = span / 12;
	int16_t w = stride * 2 / 3;
	float beat = 0.25f + 0.75f * fabsf(sinf((elapsed % 6000) * 0.0062831853f + index * 0.7f));
	int16_t h = int16_t(layout.left.h * beat);
	return {int16_t(layout.left.x + index * stride), int16_t(cfg.screenH / 2 - h / 2),
		w, h, int16_t((w < h ? w : h) / 2), index < 6};
}

/** @brief Four semantic equalizer bands. @param index Column. @return RGB565 ink. */
inline uint16_t eye_equalizer_ink(uint8_t index) {
	static constexpr uint16_t colors[] = {eye_rgb(100, 225, 200), eye_rgb(100, 190, 245),
		eye_rgb(175, 130, 245), eye_rgb(245, 135, 190)};
	return colors[index / 3];
}

/** @brief Morph eyes into colored equalizer columns. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_equalizer(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed, float a) {
	for (uint8_t i = 0; i < 12; ++i) {
		if (i == 2 || i == 9) { continue; }
		EyeBox bar = eye_equalizer_bar(cfg, layout, i, elapsed);
		gfx->fillRoundRect(bar.x, bar.y, bar.w, bar.h, bar.radius,
			eye_mix(eye_equalizer_ink(i), cfg.bgColor, uint8_t(a * a * 100)));
	}
	// Paint morphing eyes last: background columns must never cut slots into them.
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &eye : eyes) {
		uint8_t index = eye.left ? 2 : 9;
		EyeConfig body = cfg;
		body.fgColor = eye_mix(eye_equalizer_ink(index), cfg.fgColor, uint8_t(a * 100));
		eye_scene_body(gfx, body, eye_scene_box(eye, eye_equalizer_bar(cfg, layout, index, elapsed), a), a);
	}
}

/** @brief Reversible red eye rings and orbiting dots, never pupil/highlight marks. @param gfx Canvas. @param cfg Config. @param b Eye. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_rotor(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
	int16_t r = (b.w < b.h ? b.w : b.h) / 2;
	EyeBox circle = {int16_t(cx - r), int16_t(cy - r), int16_t(r * 2), int16_t(r * 2), r, b.left};
	cfg.fgColor = eye_performance_ink(cfg, b.left, elapsed);
	eye_scene_body(gfx, cfg, eye_scene_box(b, circle, a), a);
	gfx->fillCircle(cx, cy, int16_t(r * 0.66f * a), cfg.bgColor);
	uint16_t ink = eye_mix(eye_rgb(255, 210, 165), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 12; ++i) {
		float angle = i * 0.523598776f +
			(elapsed % 6000) * (b.left ? 0.0020943951f : -0.0020943951f);
		int16_t radius = int16_t(r * 1.2f);
		int16_t x = int16_t(cx + cosf(angle) * radius), y = int16_t(cy + sinf(angle) * radius);
		gfx->fillCircle(x, y, int16_t((r / 12 + 1) * a), ink);
	}
}

/** @brief Render full-body transformations, leaving other moods to the style renderer. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry time. @return Handled. */
inline bool eye_performance_scene(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed) {
	if (!eye_performance(cfg.expression)) { return false; }
	float a = eye_performance_amount(elapsed);
	if (a == 0) { return false; }
	if (cfg.expression == DANCING) {
		eye_performance_equalizer(gfx, cfg, layout, elapsed, a); return true;
	}
	if (cfg.expression == SUSHI) {
		eye_performance_sushi(gfx, cfg, layout, elapsed, a); return true;
	}
	const EyeBox eyes[] = {layout.left, layout.right};
	if (cfg.expression != ENERGETIC && cfg.expression != MENACING) { return false; }
	for (const EyeBox &b : eyes) {
		if (cfg.expression == ENERGETIC) { eye_performance_turbo(gfx, cfg, b, elapsed, a); }
		else { eye_performance_rotor(gfx, cfg, b, elapsed, a); }
	}
	return true;
}

/** @brief Rain streams start at the top edge and fall across the canvas, not from the eyes. @param gfx Canvas. @param cfg Config. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_rain(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	uint32_t phase = elapsed % 6000;
	if (phase < 600) { return; }
	int16_t tail = cfg.screenH / 35 + 2;
	uint16_t ink = eye_mix(eye_rgb(80, 180, 245), cfg.bgColor, uint8_t(a * 85));
	for (uint8_t i = 0; i < 12; ++i) {
		if (phase - 600 < i * 80u) { continue; }
		uint32_t age = (phase - 600 - i * 80u) % 1300;
		int16_t x = 2 + ((i * 71 + 11) % 127) * (cfg.screenW - 4) / 127;
		int16_t head = int32_t(age) * (cfg.screenH + tail) / 1300;
		int16_t top = eye_limit(head - tail, 0, cfg.screenH - 1);
		int16_t bottom = eye_limit(head, top, cfg.screenH - 1);
		gfx->drawLine(x, top, x, bottom, ink);
	}
}

/** @brief Draw splash droplets or outward spray, without particle storage. @param gfx Canvas. @param cfg Config. @param b Eye. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	uint16_t ink = eye_mix(eye_rgb(80, 180, 245), cfg.bgColor, uint8_t(a * 85));
	int16_t scale = eye_scale(cfg);
	for (uint8_t i = 0; i < 6; ++i) {
		float t = ((elapsed % 6000 + i * 137) % 1000) / 1000.0f;
		int16_t r = int16_t((2 + i % 2) * scale / 100);
		if (r < 1) { r = 1; }
		int16_t x = b.x + b.w * (i + 1) / 7;
		int16_t y = b.y + b.h + int16_t(t * cfg.screenH / 7);
		if (cfg.expression == SPRAYING) {
			x = int16_t((b.left ? b.x : b.x + b.w) + (b.left ? -1 : 1) * t * cfg.screenW / 8);
			y = int16_t(b.y + b.h / 2 + (int(i) - 3) * t * scale / 10);
		}
		if (cfg.expression == SPLASH) { x += int16_t(sinf(i * 1.2f + t * 6.2831853f) * b.w / 4); }
		x = eye_limit(x, r, cfg.screenW - r - 1);
		y = eye_limit(y, r, cfg.screenH - r - 1);
		gfx->fillCircle(x, y, r, ink);
		gfx->fillTriangle(x - r, y, x + r, y, x, y - r * 2, ink);
	}
	if (cfg.expression != SPLASH) { return; }
	for (int16_t x = b.x; x < b.x + b.w; ++x) {
		int16_t y = int16_t(b.y + b.h + sinf(x * 0.15f + (elapsed % 6000) * 0.0062831853f) * scale / 30);
		gfx->fillRect(x, y, 1, scale / 40 + 1, ink);
	}
}

/** @brief Draw bounded puffs, bubbles, pendulum strings or soft rays. @param gfx Canvas. @param cfg Config. @param b Eye. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_air(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, scale = eye_scale(cfg);
	uint16_t ink = eye_mix(eye_performance_ink(cfg, b.left, elapsed), cfg.bgColor, uint8_t(a * 45));
	if (cfg.expression == SWINGING) {
		gfx->drawLine(cfg.screenW * (b.left ? 1 : 3) / 4,
			eye_limit(b.y - cfg.screenH / 6, 0, cfg.screenH - 1), cx, b.y, ink);
	} else if (cfg.expression == SMOKE || cfg.expression == SICK) {
		for (uint8_t i = 0; i < 3; ++i) {
			float t = ((elapsed % 6000 + i * 500) % 1500) / 1500.0f;
			int16_t r = int16_t((cfg.expression == SICK ? 3 : 5 + 5 * t) * scale / 100);
			if (r < 1) { r = 1; }
			int16_t x = eye_limit(int16_t(cx + sinf(t * 6.2831853f + i) * b.w / 3), r, cfg.screenW - r - 1);
			int16_t y = eye_limit(int16_t(b.y - t * cfg.screenH / 7), r, cfg.screenH - r - 1);
			uint16_t puff = eye_mix(ink, cfg.bgColor, uint8_t((1 - t) * 100));
			if (cfg.expression == SICK) { gfx->drawCircle(x, y, r, puff); }
			else { gfx->fillCircle(x, y, r, puff); }
		}
	} else if (cfg.expression == BLINDING || cfg.expression == BUZZING) {
		for (uint8_t i = 0; i < 6; ++i) {
			float angle = i * 1.04719755f + (elapsed % 6000) * 0.00104719755f;
			int16_t rx = b.w / 2 + scale / 10, ry = b.h / 2 + scale / 10;
			gfx->drawLine(int16_t(cx + cosf(angle) * rx), int16_t(cy + sinf(angle) * ry),
				int16_t(cx + cosf(angle) * (rx + scale / 12)),
				int16_t(cy + sinf(angle) * (ry + scale / 12)), ink);
		}
	}
}

/** @brief Decorations stay behind the eye bodies and disappear before the cycle ends. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry time. */
inline void eye_performance_background(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed) {
	if (!eye_performance(cfg.expression)) { return; }
	float a = eye_performance_amount(elapsed);
	if (a == 0) { return; }
	if (cfg.expression == STARGAZING) {
		eye_performance_stars(gfx, cfg, elapsed, a * eye_stargazing_look(elapsed)); return;
	}
	if (cfg.expression == RAIN) { eye_performance_rain(gfx, cfg, elapsed, a); return; }
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &b : eyes) {
		if (cfg.expression == SPLASH || cfg.expression == SPRAYING) {
			eye_performance_water(gfx, cfg, b, elapsed, a);
		} else { eye_performance_air(gfx, cfg, b, elapsed, a); }
	}
}

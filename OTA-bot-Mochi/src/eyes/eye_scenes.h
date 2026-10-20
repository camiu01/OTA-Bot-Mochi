/**
 * @file eye_scenes.h
 * @brief Reversible eye-to-scene morphs with an entry-relative animation clock.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_effects.h"
#include "../play/mini_widgets.h"

/** @brief Restart an effect when the rendered mood changes. @param st State. @param expression Rendered mood. @return Entry-relative time. */
inline uint32_t eye_effect_elapsed(EyeState &st, Expression expression) {
	if (!st.effectClockInit || st.effectExpression != expression) {
		st.effectExpression = expression;
		st.effectStartMs = st.lastMs;
		st.effectClockInit = true;
	}
	return st.lastMs - st.effectStartMs;
}

/** @brief Twelve-second loop: eyes, morph, scene, reverse morph, eyes. @param elapsed Time since entry. @return Smooth scene fraction. */
inline float eye_scene_amount(uint32_t elapsed) {
	uint32_t phase = elapsed % 12000;
	float t = phase < 800 ? 0 : (phase < 2000 ? (phase - 800) / 1200.0f :
		(phase < 10000 ? 1 : (phase < 11200 ? (11200 - phase) / 1200.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Interpolate one bounded box without changing its side. @param a Eye. @param b Target. @param t Fraction. @return Morphed box. */
inline EyeBox eye_scene_box(const EyeBox &a, const EyeBox &b, float t) {
	return {int16_t(a.x + (b.x - a.x) * t), int16_t(a.y + (b.y - a.y) * t),
		int16_t(a.w + (b.w - a.w) * t), int16_t(a.h + (b.h - a.h) * t),
		int16_t(a.radius + (b.radius - a.radius) * t), a.left};
}

/** @brief Preserve the initial style then soften it toward the scene shape. @param gfx Canvas. @param cfg Config. @param box Shape. @param t Morph fraction. */
inline void eye_scene_body(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &box, float t) {
	cfg.slant = int8_t(cfg.slant * (1 - t));
	if (cfg.style == TRAPEZ && t < 0.5f) { eye_body(gfx, cfg, box); }
	else { gfx->fillRoundRect(box.x, box.y, box.w, box.h, box.radius, cfg.fgColor); }
	if (t < 0.3f) {
		cfg.pupilColor = eye_mix(cfg.fgColor, cfg.pupilColor, uint8_t(t * 333));
		cfg.highlightColor = eye_mix(cfg.fgColor, cfg.highlightColor, uint8_t(t * 333));
		eye_pupils(gfx, cfg, EyeState{}, box);
	}
}

/** @brief Merge both eyes into a hollow dial, then restore them along the same path. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry-relative time. @param t Scene fraction. */
inline void eye_scene_gauge(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	EyeBox dial = eye_gauge_box(cfg);
	dial.radius = dial.w / 2;
	float reveal = t < 0.65f ? 0 : (t - 0.65f) / 0.35f;
	EyeConfig body = cfg;
	body.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t((1 - reveal) * 100));
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &eye : eyes) {
		EyeBox box = eye_scene_box(eye, dial, t);
		eye_scene_body(gfx, body, box, t);
	}
	if (reveal > 0) {
		gfx->fillCircle(dial.x + dial.w / 2, dial.y + dial.h / 2,
			int16_t((dial.w / 2 - 2) * reveal), cfg.bgColor);
		EyeConfig ink = cfg;
		ink.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(reveal * 100));
		eye_gauge_draw(gfx, ink, elapsed, uint8_t(reveal * 100));
	}
}

/** @brief Turn eyes into paddles and back without affecting the playable desk game. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry-relative time. @param t Scene fraction. */
inline void eye_scene_pong(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	static PongGame demo;
	if (elapsed % 12000 < 2000) { demo.reset(elapsed); }
	else if (t == 1) { demo.tick(elapsed, true); }
	int16_t x = layout.left.x, y = layout.left.y;
	int16_t w = layout.right.x + layout.right.w - x;
	int16_t h = layout.left.h > layout.right.h ? layout.left.h : layout.right.h;
	uint8_t opacity = uint8_t(t * t * 100);
	mini_pong_draw(gfx, demo, x, y, w, h, eye_mix(cfg.fgColor, cfg.bgColor, opacity));
	EyeBox targets[] = {
		{int16_t(x + w * 8 / 240), int16_t(y + int16_t(demo.left - 16) * h / 160),
			int16_t(w * 4 / 240 + 1), int16_t(h * 32 / 160), 0, true},
		{int16_t(x + w * 228 / 240), int16_t(y + int16_t(demo.right - 16) * h / 160),
			int16_t(w * 4 / 240 + 1), int16_t(h * 32 / 160), 0, false}
	};
	eye_scene_body(gfx, cfg, eye_scene_box(layout.left, targets[0], t), t);
	eye_scene_body(gfx, cfg, eye_scene_box(layout.right, targets[1], t), t);
}

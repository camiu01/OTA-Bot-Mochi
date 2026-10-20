/**
 * @file eye_performances.h
 * @brief Original colored eye performances: bounded geometry and reversible six-second cycles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_effects.h"

/** @brief Identify the appended performance moods without changing older IDs. @param expr Mood. @return Performance. */
inline bool eye_performance(Expression expr) { return expr >= DANCING && expr <= STARGAZING; }

/** @brief Eyes, eased performance, reverse, eyes. @param elapsed Entry-relative time. @return Strength, 0..1. */
inline float eye_performance_amount(uint32_t elapsed) {
	uint32_t p = elapsed % 6000;
	float t = p < 600 ? 0 : (p < 1500 ? (p - 600) / 900.0f :
		(p < 4500 ? 1 : (p < 5400 ? (5400 - p) / 900.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Look up after settling at the bottom; lower the gaze before returning. @param elapsed Entry time. @return Upward gaze, 0..1. */
inline float eye_stargazing_look(uint32_t elapsed) {
	uint32_t p = elapsed % 6000;
	float t = p < 1500 ? 0 : (p < 2100 ? (p - 1500) / 600.0f :
		(p < 3900 ? 1 : (p < 4500 ? (4500 - p) / 600.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Frame-local semantic color; the selected ink returns at both endpoints. @param cfg Config. @param left Side. @param elapsed Entry time. @return RGB565 ink. */
inline uint16_t eye_performance_ink(const EyeConfig &cfg, bool left, uint32_t elapsed) {
	static constexpr uint16_t inks[] = {
		eye_rgb(120, 220, 255), eye_rgb(245, 70, 90), eye_rgb(80, 175, 245),
		eye_rgb(120, 235, 150), eye_rgb(190, 135, 245), eye_rgb(255, 205, 110),
		eye_rgb(255, 200, 85), eye_rgb(245, 225, 90), eye_rgb(155, 215, 105),
		eye_rgb(70, 185, 245), eye_rgb(100, 225, 240), eye_rgb(245, 235, 215),
		eye_rgb(120, 175, 210), eye_rgb(130, 195, 195), eye_rgb(240, 90, 110),
		eye_rgb(255, 175, 120), eye_rgb(130, 225, 210), eye_rgb(125, 185, 245),
		eye_rgb(245, 165, 205), eye_rgb(175, 150, 245), eye_rgb(150, 190, 245)
	};
	if (!eye_performance(cfg.expression)) { return cfg.fgColor; }
	uint16_t ink = inks[uint8_t(cfg.expression) - uint8_t(DANCING)];
	if (cfg.expression == POLICE) {
		float wave = 0.5f + 0.5f * sinf((elapsed % 6000) * 0.0020943951f);
		if (!left) { wave = 1 - wave; }
		ink = eye_mix(eye_rgb(245, 70, 90), eye_rgb(85, 145, 245), uint8_t(wave * 100));
	}
	return eye_mix(ink, cfg.fgColor, uint8_t(eye_performance_amount(elapsed) * 100));
}

/** @brief Apply one original motion in reference-eye units. @param expr Mood. @param left Side. @param phase Loop time. @param a Strength. @param sx Width multiplier. @param sy Height multiplier. @param dx Horizontal shift. @param dy Vertical shift. */
inline void eye_performance_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy) {
	float wave = sinf(phase * 0.0062831853f);
	switch (expr) {
		case GROWING: sx += a * (0.22f + 0.08f * wave); sy += a * 0.22f; break;
		case SHRINK: sx -= a * 0.5f; sy -= a * 0.58f; break;
		case SWINGING: dx = a * 15 * wave; dy = a * (6 - 6 * wave * wave); break;
		case BLINDING: sy -= a * 0.78f; sx += a * 0.08f; break;
		case BUZZING: dx = a * 3 * sinf(phase * 0.031415927f); dy = a * 2 * cosf(phase * 0.06283185f); break;
		case SICK: sy -= a * 0.45f; dy = a * (7 + 4 * wave); dx = a * 5 * wave; break;
		case RAIN: sy -= a * 0.28f; dy = a * 5; break;
		case SPLASH: sy -= a * (0.3f + 0.15f * wave); dy = a * 6 * wave; break;
		case SPRAYING: sx -= a * 0.28f; sy -= a * 0.1f; break;
		case SMOKE: sy -= a * 0.25f; dx = a * 3 * wave; break;
		case HELLO: dy = -a * 7 * fabsf(wave); if (!left) { sy -= a * 0.8f * (0.5f + 0.5f * wave); } break;
		case DISTRACTED: dx = a * 14 * sinf(phase * 0.0020943951f + (left ? 0 : 1.3f)); dy = a * (left ? -5 : 5); break;
		case LOOK_LEFT: dx = -a * 18; break;
		case LOOK_RIGHT: dx = a * 18; break;
		case LOOK_DOWN: dy = a * 16; sy -= a * 0.15f; break;
		default: break;
	}
}

/** @brief Transform both eyes and clamp the final rectangles to the actual canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry time. */
inline void eye_performance_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed) {
	if (!eye_performance(cfg.expression)) { return; }
	float a = eye_performance_amount(elapsed);
	if (a == 0) { return; }
	EyeBox *boxes[] = {&layout.left, &layout.right};
	for (EyeBox *b : boxes) {
		if (cfg.expression == STARGAZING) {
			float look = eye_stargazing_look(elapsed);
			int16_t w = eye_limit(b->w * 35 / 100, 6, cfg.screenW);
			int16_t h = eye_limit(int16_t(b->h * (0.35f + 0.035f * look)), 4, cfg.screenH);
			int16_t cx = cfg.screenW / 2 + (b->left ? -1 : 1) * (w / 2 + cfg.screenW / 40);
			int16_t y = cfg.screenH - cfg.screenH / 20 - h - int16_t(look * cfg.screenH / 40);
			b->x += int16_t((cx - w / 2 - b->x) * a);
			b->y += int16_t((y - b->y) * a);
			b->w += int16_t((w - b->w) * a); b->h += int16_t((h - b->h) * a);
			b->radius = eye_limit(b->radius, 0, (b->w < b->h ? b->w : b->h) / 2);
			continue;
		}
		float sx = 1, sy = 1, dx = 0, dy = 0;
		eye_performance_motion(cfg.expression, b->left, elapsed % 6000, a, sx, sy, dx, dy);
		int16_t cx = b->x + b->w / 2, cy = b->y + b->h / 2;
		b->w = eye_limit(int16_t(b->w * sx), 6, cfg.screenW);
		b->h = eye_limit(int16_t(b->h * sy), 4, cfg.screenH);
		b->x = eye_limit(int16_t(cx - b->w / 2 + dx * eye_scale(cfg) / 100), 0, cfg.screenW - b->w);
		b->y = eye_limit(int16_t(cy - b->h / 2 + dy * eye_scale(cfg) / 100), 0, cfg.screenH - b->h);
		b->radius = eye_limit(b->radius, 0, (b->w < b->h ? b->w : b->h) / 2);
	}
}

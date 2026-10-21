/**
 * @file eye_reactions.h
 * @brief Eye-only procedural reactions informed by local GIF motion references.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_effects.h"
#include "eye_crying.h"
#include <math.h>

/** @brief Settle, react, then return within the ordinary three-second mood cycle. @param elapsed Entry-relative milliseconds. @return Eased strength, 0..1. */
inline float eye_reaction_amount(uint32_t elapsed) {
	uint32_t p = elapsed % 3000;
	float t = p < 300 ? 0 : (p < 800 ? (p - 300) / 500.0f :
		(p < 2200 ? 1 : (p < 2700 ? (2700 - p) / 500.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Fade reference-inspired reaction colors in and out without changing preferences. @param cfg Selected palette and mood. @param elapsed Entry-relative time. @return Frame-local body ink. */
inline uint16_t eye_reaction_ink(const EyeConfig &cfg, uint32_t elapsed) {
	uint16_t ink = cfg.fgColor;
	switch (cfg.expression) {
		case LOVE: ink = eye_rgb(255, 92, 150); break;
		case CRYING: ink = eye_rgb(75, 205, 255); break;
		case ANGRY: case GRUMPY: ink = eye_rgb(255, 135, 65); break;
		case FURIOUS: ink = eye_rgb(245, 70, 85); break;
		default: break;
	}
	return eye_mix(ink, cfg.fgColor, uint8_t(eye_reaction_amount(elapsed) * 100));
}

/** @brief Resolve expression-specific motion without changing the selected palette. @param expr Mood. @param left Eye side. @param phase Loop milliseconds. @param a Strength. @param sx Width scale. @param sy Height scale. @param dx X shift. @param dy Y shift. */
inline void eye_reaction_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy) {
	float wave = sinf(phase * 0.0062831853f);
	switch (expr) {
		case HAPPY: case MILD_HAPPY: case GRIN:
			dy = -a * (3 + 2 * wave); sx += a * 0.05f; break;
		case GLEE: case BEAMING:
			dy = -a * 5 * fabsf(wave); sx += a * 0.12f; sy -= a * 0.15f; break;
		case ANGRY: case GRUMPY: case FURIOUS:
			sy -= a * (expr == FURIOUS ? 0.3f : 0.15f);
			dx = a * (expr == FURIOUS ? 2 : 1) * sinf(phase * 0.025132741f); break;
		case CRYING:
			sx += 0.28f * eye_crying_closure(phase);
			dy = a * 3 * sinf(phase * 0.0041887902f); break;
		case SLEEPY: case DOZE:
			dy = a * (4 + 2 * wave); break;
		case SQUINT: sy -= a * 0.7f; sx += a * 0.08f; break;
		case SMIRK: case SMUG: case SASSY:
			sy -= a * (left ? 0.1f : 0.5f); dy = a * (left ? -4 : 3); break;
		case SCARED: case TERRIFIED: case NERVOUS:
			dx = a * 3 * sinf(phase * 0.031415927f + (left ? 0 : 1.5707963f));
			sy += a * 0.08f; break;
		case SURPRISED: case SHOCKED: case AWE:
			sx += a * 0.12f; sy += a * 0.15f; dy = -a * 5; break;
		case LOVE:
			sx += a * 0.12f * wave; sy += a * 0.12f * wave; break;
		case SWIRLY: dx = a * 5 * wave; dy = a * (left ? 3 : -3) * wave; break;
		case SNEEZE:
			if (phase < 1100) { sx -= a * 0.1f; sy += a * 0.18f; dy = -a * 4; }
			else if (phase < 2000) { sx += a * 0.15f; sy -= a * 0.4f; dy = a * 7; }
			else { dy = -a * 3; }
			break;
		default: break;
	}
}

/** @brief Apply bounded local reactions after the ordinary live pose. @param cfg Config. @param layout Eye pair. @param elapsed Entry-relative milliseconds. */
inline void eye_reaction_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed) {
	float a = eye_reaction_amount(elapsed);
	if (a == 0) { return; }
	EyeBox *boxes[] = {&layout.left, &layout.right};
	for (EyeBox *b : boxes) {
		float sx = 1, sy = 1, dx = 0, dy = 0;
		eye_reaction_motion(cfg.expression, b->left, elapsed % 3000, a, sx, sy, dx, dy);
		int16_t cx = b->x + b->w / 2, cy = b->y + b->h / 2;
		b->w = eye_limit(int16_t(b->w * sx), 6, cfg.screenW);
		b->h = eye_limit(int16_t(b->h * sy), 4, cfg.screenH);
		b->x = eye_limit(int16_t(cx - b->w / 2 + dx * eye_scale(cfg) / 100), 0, cfg.screenW - b->w);
		b->y = eye_limit(int16_t(cy - b->h / 2 + dy * eye_scale(cfg) / 100), 0, cfg.screenH - b->h);
		b->radius = eye_limit(b->radius, 0, (b->w < b->h ? b->w : b->h) / 2);
	}
}

/** @brief Draw a thick closed-eye chevron inside its box. @param gfx Canvas. @param cfg Config. @param b Eye. */
inline void eye_reaction_chevron(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t edge = b.left ? b.x + 2 : b.x + b.w - 3;
	int16_t tip = b.left ? b.x + b.w - 3 : b.x + 2;
	int16_t thick = eye_limit(b.w / 7, 2, b.h / 5);
	for (int16_t i = 0; i < thick; ++i) {
		gfx->drawLine(edge, b.y + b.h / 5 + i, tip, b.y + b.h / 2 + i, cfg.fgColor);
		gfx->drawLine(tip, b.y + b.h / 2 + i, edge, b.y + b.h * 4 / 5 + i, cfg.fgColor);
	}
}

/** @brief Draw a shallow closed arc, with distinct sleepy and dozing curvature. @param gfx Canvas. @param cfg Config. @param b Eye. */
inline void eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t thick = eye_limit(b.h / 8, 2, 6);
	for (int16_t x = 1; x < b.w - 1; ++x) {
		int32_t t = 2 * x - b.w + 1;
		int16_t curve = int16_t(t * t * b.h / (int32_t(b.w) * b.w * 3));
		int16_t y = cfg.expression == DOZE ? b.h * 2 / 3 - curve : b.h / 2 + curve / 2;
		gfx->fillRect(b.x + x, b.y + y, 1, thick, cfg.fgColor);
	}
}


/** @brief Render closed/reaction silhouettes without adding pupils or mouths. @param gfx Canvas. @param cfg Config. @param st Gaze. @param b Eye. @param elapsed Entry-relative time. @param neutral Selected neutral ink. @return Eye handled. */
inline bool eye_reaction_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	if (cfg.expression == CRYING) {
		eye_reaction_tears(gfx, cfg, st, b, elapsed, neutral);
		return true;
	}
	if (eye_reaction_amount(elapsed) < 0.8f) { return false; }
	switch (cfg.expression) {
		case GLEE: case BEAMING:
			eye_reaction_chevron(gfx, cfg, b); return true;
		case SNEEZE:
			if (elapsed % 3000 < 1100 || elapsed % 3000 >= 2000) { return false; }
			eye_reaction_chevron(gfx, cfg, b); return true;
		case SLEEPY: case DOZE:
			eye_reaction_rest(gfx, cfg, b); return true;
		default: return false;
	}
}

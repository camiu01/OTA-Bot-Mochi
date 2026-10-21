/**
 * @file eye_crying.h
 * @brief Neutral closed lids above attached, softly flowing blue tear curtains.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_effects.h"

/** @brief Ease a bounded phase fraction. @param t Fraction. @return Smooth 0..1 value. */
inline float eye_crying_ease(float t) {
	t = t < 0 ? 0 : (t > 1 ? 1 : t);
	return t * t * (3 - 2 * t);
}

/** @brief Blink closed, hold while water drains, then reopen. @param elapsed Entry-relative time. @return Closure, 0..1. */
inline float eye_crying_closure(uint32_t elapsed) {
	uint32_t p = elapsed % 3000;
	return eye_crying_ease(p < 250 ? 0 : (p < 650 ? (p - 250) / 400.0f :
		(p < 2500 ? 1 : (p < 2850 ? (2850 - p) / 350.0f : 0))));
}

/** @brief Draw one clean neutral eye compressing toward an upper, rounded closed lid. @param gfx Canvas. @param cfg Neutral palette. @param st Gaze. @param b Eye. @param closure Closure fraction. */
inline void eye_crying_lid(Arduino_GFX *gfx, EyeConfig cfg, const EyeState &st, const EyeBox &b, float closure) {
	int16_t lid = eye_limit(b.h / 9, 3, 9);
	EyeBox closing = b;
	closing.h = int16_t(b.h + (lid - b.h) * closure);
	int16_t center = b.y + b.h / 2;
	int16_t target = b.y + b.h / 6 + lid / 2;
	closing.y = int16_t(center + (target - center) * closure) - closing.h / 2;
	closing.radius = eye_limit(b.radius, 0, (closing.w < closing.h ? closing.w : closing.h) / 2);
	cfg.expression = NEUTRAL;
	if (closure == 1 || closing.h < 4) {
		gfx->fillRoundRect(closing.x, closing.y, closing.w, closing.h, closing.h / 2, cfg.fgColor);
	} else {
		eye_body(gfx, cfg, closing);
		if (closing.h > 13) { eye_pupils(gfx, cfg, st, closing); }
	}
}

/** @brief Length and draining tail of a curtain; old water falls away instead of retracting upward. @param b Eye. @param elapsed Time. @param top Output water origin. @param begin First water row. @param end Last water row, exclusive. */
inline void eye_crying_water_rows(const EyeBox &b, uint32_t elapsed, int16_t &top, int16_t &begin, int16_t &end) {
	uint32_t phase = elapsed % 3000;
	int16_t lid = eye_limit(b.h / 9, 3, 9);
	top = b.y + b.h / 6 + lid / 2;
	int16_t available = eye_limit(b.y + b.h - 2 - top, 0, b.h);
	float growth = eye_crying_ease(phase < 500 ? 0 : (phase - 500) / 750.0f);
	float drain = eye_crying_ease(phase < 2200 ? 0 : (phase - 2200) / 300.0f);
	begin = int16_t(available * drain);
	end = int16_t(available * growth);
}

/** @brief Draw broad, attached water with traveling edge ripples and soft downward highlights. @param gfx Canvas. @param cfg Water palette. @param b Eye. @param elapsed Time. @param neutral Neutral lid ink. */
inline void eye_crying_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	int16_t top, begin, end;
	eye_crying_water_rows(b, elapsed, top, begin, end);
	float phase = float(elapsed % 3000);
	for (int16_t row = begin; row < end; ++row) {
		float wave = sinf(row * 0.18f - phase * 0.0034906585f);
		int16_t width = int16_t(b.w * (0.84f + 0.035f * wave));
		int16_t x = b.x + (b.w - width) / 2 + int16_t(wave * b.w / 20);
		float stripe = 0.5f + 0.5f * sinf(row * 0.32f - phase * 0.008f);
		uint16_t ink = eye_mix(neutral, cfg.fgColor, uint8_t(6 + 20 * stripe * stripe * stripe));
		gfx->fillRect(x, top + row, width, 1, ink);
	}
}

/** @brief Keep neutral lids attached to blue curtains throughout a complete crying loop. @param gfx Canvas. @param cfg Water palette. @param st Gaze. @param b Eye. @param elapsed Time. @param neutral Selected neutral ink. */
inline void eye_reaction_tears(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	eye_crying_water(gfx, cfg, b, elapsed, neutral);
	EyeConfig lids = cfg;
	lids.fgColor = neutral;
	eye_crying_lid(gfx, lids, st, b, eye_crying_closure(elapsed));
}

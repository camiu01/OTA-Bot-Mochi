/**
 * @file eye_geometry.h
 * @brief Bounded geometry shared by rendering and native regression tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"

struct EyeBox { int16_t x, y, w, h, radius; bool left; };
struct EyeLayout { EyeBox left, right; };

/** @brief Clamp a geometry value. @param value Input. @param low Min. @param high Max. @return Bounded. */
inline int16_t eye_limit(int16_t value, int16_t low, int16_t high) {
	return value < low ? low : (value > high ? high : value);
}

/** @brief Resolve one eye's expression-specific dimensions. @param cfg Config. @param st State. @param left Side. @return Geometry. */
inline EyeBox eye_box(const EyeConfig &cfg, const EyeState &st, bool left) {
	int16_t w = cfg.eyeW, h = cfg.eyeH, dy = 0;
	switch (cfg.expression) {
		case SURPRISED: w += 8; h += 12; break;
		case AWE: w += 6; h += 16; break;
		case SCARED: w -= 8; h += 6; break;
		case HAPPY: h = h * 72 / 100; break;
		case SLEEPY: h = h * 35 / 100; break;
		case SQUINT: h = h * 45 / 100; break;
		case FOCUSED: h = h * 80 / 100; break;
		case EXCITED: w += 4; h += 2; break;
		case TERRIFIED: h += 18; break;
		case PANICKED: h += 14; break;
		case GRIN: w += 6; break;
		case CAT: w -= 4; break;
		case SHY: h = h * 62 / 100; dy += 4; break;
		case TIRED: h = h * 70 / 100; break;
		case GLUM: h = h * 75 / 100; break;
		case CRYING: h = h * 85 / 100; break;
		case DEADPAN: h = 12; break;
		case MILD_HAPPY: h = h * 85 / 100; break;
		case POUT: w += 4; h = h * 85 / 100; break;
		case KISS: w = w * 60 / 100; h = h * 70 / 100; break;
		case NERVOUS: w = w * 82 / 100; h = h * 88 / 100; break;
		case SHOCKED: w += 12; h += 24; break;
		case LAZY: h = h * 55 / 100; break;
		case DOZE: h = h * 22 / 100; break;
		case DAYDREAM: w += 6; h = h * 92 / 100; break;
		case GRUMPY: h = h * 80 / 100; break;
		case ZEN: h = h * 45 / 100; break;
		case MERCIFUL: h = h * 85 / 100; break;
		case MANIC: w += 10; h += 10; break;
		case BEAMING: w += 4; h = h * 70 / 100; break;
		default: break;
	}
	int16_t nominal_w = eye_limit(w, 20, 88);
	if (cfg.asym && !st.blinking) {
		if (cfg.expression == SKEPTIC) { w += left ? 4 : -8; h += left ? 6 : -16; }
		if (cfg.expression == SUSPICIOUS) { dy = left ? -5 : 5; }
		if (cfg.expression == SLEEPY_EYES) { h = h * (left ? 45 : 30) / 100; }
		if (cfg.expression == PROUD) { h = h * (left ? 85 : 70) / 100; }
		if (cfg.expression == SMUG) { h = h * (left ? 80 : 55) / 100; }
		if (cfg.expression == CONFUSED) { w += left ? 6 : -6; h += left ? 8 : -10; }
		if (cfg.expression == CURIOUS) { dy = left ? 0 : -6; }
		if (cfg.expression == WONDERING) { dy = left ? 0 : -5; }
		if (cfg.expression == ZOMBIE) { h = h * (left ? 55 : 100) / 100; }
		if (cfg.expression == WINK && !left) { h = 6; }
		if (cfg.expression == SMIRK) { h = h * (left ? 70 : 100) / 100; }
		if (cfg.expression == MELTING) { h = h * (left ? 100 : 50) / 100; dy += left ? 0 : 6; }
		if (cfg.expression == GROGGY) { h = h * (left ? 100 : 55) / 100; dy += left ? 0 : 4; }
		if (cfg.expression == SASSY) { dy += left ? -5 : 1; }
		if (cfg.expression == CHEEKY) { h = h * (left ? 100 : 18) / 100; }
		if (cfg.expression == FLIRT) { h = h * (left ? 88 : 66) / 100; dy += left ? 0 : 3; }
	}
	w = eye_limit(w, 20, 92);
	h = st.blinking ? 6 : eye_limit(h, 12, 130);
	int16_t gap = eye_limit(cfg.eyeGap, 16, 48);
	int16_t ox = eye_limit(st.saccadeX + st.tiltX, -20, 20);
	int16_t oy = eye_limit(st.saccadeY + st.tiltY + st.floatY, -18, 18);
	// Sustained lateral load rolls the pair, so cornering reads as a lean and not only a shift.
	// Kept small and mirror-symmetric; the sign is a hardware mount detail, see TODO.md.
	int16_t bank = eye_limit(st.tiltX / 3, -4, 4);
	int16_t x = (240 - 2 * nominal_w - gap) / 2 + (left ? 0 : nominal_w + gap) + ox;
	x -= (w - nominal_w) / 2;
	return {x, int16_t(140 - h / 2 + oy + dy + (left ? -bank : bank)), w, h,
		eye_limit(cfg.radius, 0, (w < h ? w : h) / 2), left};
}

/** @brief Resolve both eyes, including asymmetric expressions and blinks. @param cfg Config. @param st State. @return Pair. */
inline EyeLayout eye_layout(const EyeConfig &cfg, const EyeState &st) {
	return {eye_box(cfg, st, true), eye_box(cfg, st, false)};
}

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
		default: break;
	}
	int16_t nominal_w = eye_limit(w, 20, 88);
	if (cfg.asym && !st.blinking) {
		if (cfg.expression == SKEPTIC) { w += left ? 4 : -8; h += left ? 6 : -16; }
		if (cfg.expression == SUSPICIOUS) { dy = left ? -5 : 5; }
		if (cfg.expression == SLEEPY_EYES) { h = h * (left ? 45 : 30) / 100; }
	}
	w = eye_limit(w, 20, 92);
	h = st.blinking ? 6 : eye_limit(h, 12, 130);
	int16_t gap = eye_limit(cfg.eyeGap, 16, 48);
	int16_t ox = eye_limit(st.saccadeX + st.tiltX, -20, 20);
	int16_t oy = eye_limit(st.saccadeY + st.tiltY + st.floatY, -18, 18);
	int16_t x = (240 - 2 * nominal_w - gap) / 2 + (left ? 0 : nominal_w + gap) + ox;
	x -= (w - nominal_w) / 2;
	return {x, int16_t(140 - h / 2 + oy + dy), w, h,
		eye_limit(cfg.radius, 0, (w < h ? w : h) / 2), left};
}

/** @brief Resolve both eyes, including asymmetric expressions and blinks. @param cfg Config. @param st State. @return Pair. */
inline EyeLayout eye_layout(const EyeConfig &cfg, const EyeState &st) {
	return {eye_box(cfg, st, true), eye_box(cfg, st, false)};
}

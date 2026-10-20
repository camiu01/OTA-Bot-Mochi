/**
 * @file eye_geometry.h
 * @brief Bounded geometry shared by rendering and native regression tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"

struct EyeBox { int16_t x, y, w, h, radius; bool left; };
struct EyeLayout { EyeBox left, right; };
constexpr int16_t EYE_REF_W = 240;	/**< Reference panel width the style presets were tuned on. */
constexpr int16_t EYE_REF_H = 280;	/**< Reference panel height the style presets were tuned on. */
#ifndef EYE_FILL_EXCESS
#define EYE_FILL_EXCESS 50	/**< Percent of the extra panel ratio reinvested into the face. */
#endif

/** @brief Clamp a geometry value. @param value Input. @param low Min. @param high Max. @return Bounded. */
inline int16_t eye_limit(int16_t value, int16_t low, int16_t high) {
	return value < low ? low : (value > high ? high : value);
}

/**
 * @brief Panel fit of an eye pair versus the 240x280 reference panel.
 *
 * @details The smaller axis sets the base ratio, so a taller or narrower panel
 *          can never overflow the pair. Panels larger than the reference then
 *          grow the face faster than the frame: EYE_FILL_EXCESS percent of the
 *          ratio beyond 100 is reinvested into the eyes, because scaling with
 *          the panel alone keeps the same share of black on a much bigger
 *          screen. The result is then capped by the style's own horizontal
 *          budget: its widest expression, minimum gap, and the full gaze
 *          range must stay inside the panel, so a wide style such as BLOUB
 *          fills less than a narrow one instead of clipping or losing gaze.
 *          Host builds may override EYE_FILL_EXCESS with -D.
 * @param cfg Config. @return Percent of reference size, bounded 50..260.
 */
inline int16_t eye_scale(const EyeConfig &cfg) {
	int16_t sx = int16_t(cfg.screenW) * 100 / EYE_REF_W;
	int16_t sy = int16_t(cfg.screenH) * 100 / EYE_REF_H;
	int16_t base = sx < sy ? sx : sy;
	if (base > 100) {
		int16_t fill = int16_t(100 + (base - 100) * EYE_FILL_EXCESS / 100);
		base = int16_t(int32_t(base) * fill / 100);
	}
	// Widest expression: the largest symmetric delta is +12, asymmetric
	// expressions add up to +6 on one side; the pair plus twice the gaze
	// offset (40 reference pixels) must fit the panel width.
	int16_t raw = int16_t(cfg.eyeW) + 12 + (cfg.asym ? 6 : 0);
	int16_t nominal = eye_limit(raw, 20, 88);
	int16_t widest = eye_limit(raw, 20, 92);
	int16_t gap = eye_limit(cfg.eyeGap, 16, 48);
	int16_t fit = int16_t(int32_t(cfg.screenW) * 100 / (nominal + widest + gap + 40));
	return eye_limit(base < fit ? base : fit, 50, 260);
}

/** @brief One eye's expression-driven sizes in reference-panel pixels. */
struct EyeMeasures {
	int16_t w, h, dy, nominal, gap, ox, oy, bank, radius;
};

/** @brief Resolve one eye's expression-driven measures before the panel fit. @param cfg Config. @param st State. @param left Side. @return Unscaled measures. */
inline EyeMeasures eye_measures(const EyeConfig &cfg, const EyeState &st, bool left) {
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
		case IDEA: w += 4; h += 4; break;
		case POWER: h = h * 50 / 100; break;
		case HYPER: w += 6; h += 6; break;
		case SQUISH: w += 10; h = h * 65 / 100; break;
		case ARC_HAPPY: case ARC_SAD: h = h * 55 / 100; break;
		case SQUEEZED: h = h * 60 / 100; break;
		case TILTED: h = h * 92 / 100; dy = left ? -6 : 6; break;
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
	return {w, h, dy, nominal_w, gap, ox, oy, bank, cfg.radius};
}

/** @brief Scale measures to the actual panel and center the pair on it. @param cfg Config. @param m Unscaled measures. @param left Side. @return Geometry. */
inline EyeBox eye_box(const EyeConfig &cfg, const EyeMeasures &m, bool left) {
	int16_t scale = eye_scale(cfg);
	int16_t w = m.w * scale / 100;
	int16_t h = m.h * scale / 100;
	int16_t nominal = m.nominal * scale / 100;
	int16_t gap = m.gap * scale / 100;
	int16_t ox = m.ox * scale / 100;
	int16_t oy = m.oy * scale / 100;
	int16_t dy = m.dy * scale / 100;
	int16_t bank = m.bank * scale / 100;
	int16_t x = (int16_t(cfg.screenW) - 2 * nominal - gap) / 2 + (left ? 0 : nominal + gap) + ox;
	x -= (w - nominal) / 2;
	int16_t y = int16_t(cfg.screenH) / 2 - h / 2 + oy + dy + (left ? -bank : bank);
	int16_t radius = m.radius * scale / 100;
	return {x, y, w, h, eye_limit(radius, 0, (w < h ? w : h) / 2), left};
}

/** @brief Resolve both eyes, including asymmetric expressions and blinks. @param cfg Config. @param st State. @return Pair. */
inline EyeLayout eye_layout(const EyeConfig &cfg, const EyeState &st) {
	EyeMeasures left = eye_measures(cfg, st, true);
	EyeMeasures right = eye_measures(cfg, st, false);
	return {eye_box(cfg, left, true), eye_box(cfg, right, false)};
}

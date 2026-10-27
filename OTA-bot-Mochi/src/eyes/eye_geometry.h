/**
 * @file eye_geometry.h
 * @brief Bounded geometry shared by rendering and native regression tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include "face_presets.h"

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

/** @brief Apply one style-relative eye definition before the unchanged panel fit. */
inline EyeMeasures eye_preset_measures(const EyeConfig &cfg, const EyeState &st, bool left, const FacePreset *preset) {
	const FaceEye neutral{};
	const FaceEye &eye = preset ? (left ? preset->left : preset->right) : neutral;
	int16_t w = cfg.eyeW * face_percent(eye.widthScale) / 100 + eye.widthDelta;
	int16_t h = cfg.eyeH * face_percent(eye.heightScale) / 100 + eye.heightDelta;
	if (eye.heightFromWidth) { h = w; }
	if (eye.height) { h = eye.height; }
	h = h * face_percent(eye.openness) / 100;
	int16_t dy = eye.offsetY, nominal = eye_limit(w, 20, 88);
	if (cfg.asym && !st.blinking) {
		w = w * face_percent(eye.asym.widthScale) / 100 + eye.asym.widthDelta;
		h = h * face_percent(eye.asym.heightScale) / 100 + eye.asym.heightDelta;
		if (eye.asym.height) { h = eye.asym.height; }
		dy += eye.asym.offsetY;
	}
	w = eye_limit(w, 20, 92);
	h = st.blinking ? 6 : eye_limit(h, 12, 130);
	int16_t gap = eye_limit(cfg.eyeGap, 16, 48);
	int16_t gazeX = preset ? int16_t(preset->gazeX * 20) : 0;
	int16_t gazeY = preset ? int16_t(preset->gazeY * 18) : 0;
	int16_t ox = eye_limit(st.saccadeX + st.tiltX + gazeX, -20, 20);
	int16_t oy = eye_limit(st.saccadeY + st.tiltY + st.floatY + gazeY, -18, 18);
	int16_t bank = eye_limit(st.tiltX / 3, -4, 4);
	return {w, h, dy, nominal, gap, ox, oy, bank, cfg.radius};
}

/** @brief Resolve the selected face before panel scaling. */
inline EyeMeasures eye_measures(const EyeConfig &cfg, const EyeState &st, bool left) {
	return eye_preset_measures(cfg, st, left, face_preset(cfg.expression));
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

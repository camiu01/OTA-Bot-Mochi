/**
 * @file eye_backdrop.h
 * @brief Keep outgoing themed scenery alive independently of the eye handoff.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_backgrounds.h"

/** @brief Only the persistent themed painters participate; legacy eye-color effects stay separate. */
inline bool eye_themed_backdrop(Expression expression) {
	const FacePreset *p = face_preset(expression);
	return p && p->background && p->bounds == face_backgrounds::full_bounds;
}

/** @brief Preserve the last backdrop until its full 1.2-second exit has completed. */
inline void eye_backdrop_step(EyeBackdropState &bg, const EyeConfig &cfg, uint32_t now, uint32_t elapsed) {
	if (!bg.initialized) {
		bg.config = cfg; bg.since = now - elapsed; bg.initialized = true;
	}
	if (bg.leaving && uint32_t(now - bg.since) >= 1200) {
		bg.config = cfg; bg.since += 1200; bg.leaving = false;
	}
	if (bg.leaving || bg.config.expression == cfg.expression) { return; }
	if (eye_themed_backdrop(bg.config.expression)) {
		bg.frozenElapsed = now - bg.since;
		bg.since = now; bg.leaving = true;
	} else {
		bg.config = cfg; bg.since = now;
	}
}

/** @brief Draw outgoing scenery with full dirty bounds; never fade foreground eye ink. */
inline bool eye_backdrop_draw(Arduino_GFX *gfx, EyeConfig &cfg, EyeState &st,
		const EyeLayout &layout, uint32_t elapsed, bool live) {
	EyeState effects = st;
	effects.lastMs = elapsed; effects.backdropOpacity = 1;
	if (!live) {
		st.backdrop = EyeBackdropState{};
		eye_effect_background(gfx, cfg, effects, layout);
		return false;
	}
	eye_backdrop_step(st.backdrop, cfg, st.lastMs, elapsed);
	EyeBackdropState &bg = st.backdrop;
	if (bg.leaving) {
		float t = (st.lastMs - bg.since) / 1200.0f;
		effects.lastMs = bg.frozenElapsed;
		effects.backdropOpacity = 1 - t * t * (3 - 2 * t);
		EyeConfig outgoing = bg.config;
		eye_effect_background(gfx, outgoing, effects, eye_layout(outgoing, EyeState{}));
	} else if (eye_themed_backdrop(cfg.expression)) {
		effects.lastMs = st.lastMs - bg.since;
		bg.config = cfg;
		eye_effect_background(gfx, cfg, effects, layout);
	}
	if (!eye_themed_backdrop(cfg.expression)) {
		effects.lastMs = elapsed; effects.backdropOpacity = 1;
		eye_effect_background(gfx, cfg, effects, layout);
	}
	return bg.leaving;
}

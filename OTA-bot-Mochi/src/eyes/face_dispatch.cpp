/**
 * @file face_dispatch.cpp
 * @brief Generic rendering stages dispatch to the selected face's procedures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "face_support.h"
#include "faces/crying.h"
#include "faces/glow.h"
#include "faces/headlights.h"
#include "faces/gauge.h"
#include "faces/pong.h"
#include "faces/dancing.h"
#include "faces/menacing.h"
#include "faces/rain.h"
#include "faces/sushi.h"
#include "faces/energetic.h"
#include "faces/stargazing.h"
#include "faces/adore.h"
#include "faces/sleepy.h"
#include "faces/splash.h"
#include "faces/swinging.h"
#include "faces/mistake.h"
#include "faces/laughing.h"

/** @brief Blend the face's optional temporary reaction ink with the configured ink. */
uint16_t eye_reaction_ink(const EyeConfig &cfg, uint32_t elapsed) {
	const FacePreset *p = face_preset(cfg.expression);
	uint16_t ink = p && p->hasReactionInk ? p->reactionInk : cfg.fgColor;
	return eye_mix(ink, cfg.fgColor, uint8_t(eye_reaction_amount(elapsed) * 100));
}

/** @brief Dispatch ordinary emotion motion without changing clocks or clamping. */
void eye_reaction_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy) {
	const FacePreset *p = face_preset(expr);
	if (p && p->reactionMotion) { p->reactionMotion(left, phase, a, sx, sy, dx, dy); }
}

/** @brief Render a face-owned reaction silhouette when it claims the eye. */
bool eye_reaction_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st,
		const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	const FacePreset *p = face_preset(cfg.expression);
	return p && p->reactionBody && p->reactionBody(gfx, cfg, st, b, elapsed, neutral);
}

/** @brief Render a face-owned reference silhouette when it claims the eye. */
bool eye_reference_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b) {
	const FacePreset *p = face_preset(cfg.expression);
	return p && p->referenceBody && p->referenceBody(gfx, cfg, st, b, 0, cfg.fgColor);
}

/** @brief Legacy entry point for the three reference lid definitions. */
void eye_reference_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (cfg.expression != ARC_HAPPY && cfg.expression != ARC_SAD && cfg.expression != TILTED) { return; }
	const FacePreset *p = face_preset(cfg.expression);
	if (p && p->lids) { p->lids(gfx, cfg, b); }
}

/** @brief Identify a six-second performance through its definition. */
bool eye_performance(Expression expr) {
	const FacePreset *p = face_preset(expr);
	return p && p->performance;
}

/** @brief Resolve frame-local performance ink without changing the saved palette. */
uint16_t eye_performance_ink(const EyeConfig &cfg, bool left, uint32_t elapsed) {
	const FacePreset *p = face_preset(cfg.expression);
	if (!p || !p->performance) { return cfg.fgColor; }
	uint16_t ink = p->performanceColor ? p->performanceColor(cfg, left, elapsed) : p->performanceInk;
	return eye_mix(ink, cfg.fgColor, uint8_t(eye_performance_amount(elapsed) * 100));
}

/** @brief Dispatch performance motion in the existing reference units. */
void eye_performance_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy) {
	const FacePreset *p = face_preset(expr);
	if (p && p->performanceMotion) { p->performanceMotion(left, phase, a, sx, sy, dx, dy); }
}

/** @brief Apply bounded performance geometry after the live pose. */
void eye_performance_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed) {
	const FacePreset *p = face_preset(cfg.expression);
	if (!p || !p->performance) { return; }
	float a = eye_performance_amount(elapsed);
	if (a == 0) { return; }
	EyeBox *boxes[] = {&layout.left, &layout.right};
	for (EyeBox *b : boxes) {
		if (p->layout) { p->layout(cfg, *b, elapsed, a); continue; }
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

/** @brief Preserve the legacy rest-shape entry point. */
void eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	face_defs::sleepy_face::local_eye_reaction_rest(gfx, cfg, b);
}

/** @brief Preserve the legacy water-effect entry point. */
void eye_performance_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, float a) {
	face_defs::splash_face::local_eye_performance_water(gfx, cfg, b, elapsed, a);
}

/** @brief Preserve the legacy air-effect entry point. */
void eye_performance_air(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, float a) {
	face_defs::swinging_face::local_eye_performance_air(gfx, cfg, b, elapsed, a);
}

/** @brief Identify the existing appended scenery range. */
bool eye_extra_mood(Expression expr) { return expr >= MISTAKE && expr <= ENRAGED; }

/** @brief Preserve the legacy banner entry point. */
void eye_extra_banner(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	face_defs::mistake_face::local_eye_extra_banner(gfx, cfg, elapsed, a);
}

/** @brief Preserve the legacy marks entry point. */
void eye_extra_marks(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	face_defs::laughing_face::local_eye_extra_marks(gfx, cfg, layout, elapsed, a);
}

/** @brief Route the legacy scenery helper to face-owned implementations. */
void eye_extra_background(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	if (cfg.expression == MISTAKE || cfg.expression == ENCOURAGEMENT) { eye_extra_banner(gfx, cfg, elapsed, a); }
	else if (cfg.expression == ADORE) { eye_extra_hearts(gfx, cfg, elapsed, a); }
	else { eye_extra_marks(gfx, cfg, layout, elapsed, a); }
}

/** @brief Render face-owned full scenes while preserving the performance clock. */
bool eye_performance_scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed) {
	const FacePreset *p = face_preset(cfg.expression);
	if (!p || !p->performanceScene) { return false; }
	float a = eye_performance_amount(elapsed);
	return a != 0 && p->performanceScene(gfx, cfg, layout, elapsed, a);
}

/** @brief Draw scenery behind the eye pair. */
void eye_performance_background(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed) {
	const FacePreset *p = face_preset(cfg.expression);
	if (!p || !p->scenery) { return; }
	float a = eye_performance_amount(elapsed);
	if (a != 0) { p->scenery(gfx, cfg, layout, elapsed, a); }
}

/** @brief Preserve the previous effect-background entry point. */
void eye_effect_background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	const FacePreset *p = face_preset(cfg.expression);
	if (p && p->background) { p->background(gfx, cfg, st, layout); }
}

/** @brief Ask the face for additional dirty bounds, if needed. */
EyeLayout eye_effect_bounds(const EyeConfig &cfg, EyeLayout layout) {
	const FacePreset *p = face_preset(cfg.expression);
	return p && p->bounds ? p->bounds(cfg, layout) : layout;
}
/** @brief Preserve the eye_crying_ease test/internal entry point. */
float eye_crying_ease(float t) {
	return face_defs::crying_face::local_eye_crying_ease(t);
}

/** @brief Preserve the eye_crying_closure test/internal entry point. */
float eye_crying_closure(uint32_t elapsed) {
	return face_defs::crying_face::local_eye_crying_closure(elapsed);
}

/** @brief Preserve the eye_crying_lid test/internal entry point. */
void eye_crying_lid(Arduino_GFX *gfx, EyeConfig cfg, const EyeState &st, const EyeBox &b, float closure) {
	face_defs::crying_face::local_eye_crying_lid(gfx, cfg, st, b, closure);
}

/** @brief Preserve the eye_crying_water_rows test/internal entry point. */
void eye_crying_water_rows(const EyeBox &b, uint32_t elapsed, int16_t &top, int16_t &begin, int16_t &end) {
	face_defs::crying_face::local_eye_crying_water_rows(b, elapsed, top, begin, end);
}

/** @brief Preserve the eye_crying_water test/internal entry point. */
void eye_crying_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	face_defs::crying_face::local_eye_crying_water(gfx, cfg, b, elapsed, neutral);
}

/** @brief Preserve the eye_reaction_tears test/internal entry point. */
void eye_reaction_tears(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	face_defs::crying_face::local_eye_reaction_tears(gfx, cfg, st, b, elapsed, neutral);
}

/** @brief Preserve the eye_glow_color test/internal entry point. */
uint16_t eye_glow_color(uint32_t now, uint16_t base) {
	return face_defs::glow_face::local_eye_glow_color(now, base);
}

/** @brief Preserve the eye_headlight test/internal entry point. */
void eye_headlight(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t now) {
	face_defs::headlights_face::local_eye_headlight(gfx, cfg, b, now);
}

/** @brief Preserve the eye_gauge_box test/internal entry point. */
EyeBox eye_gauge_box(const EyeConfig &cfg) {
	return face_defs::gauge_face::local_eye_gauge_box(cfg);
}

/** @brief Preserve the eye_gauge_sweep test/internal entry point. */
float eye_gauge_sweep(uint32_t elapsed) {
	return face_defs::gauge_face::local_eye_gauge_sweep(elapsed);
}

/** @brief Preserve the eye_gauge_draw test/internal entry point. */
void eye_gauge_draw(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t now, uint8_t opacity) {
	face_defs::gauge_face::local_eye_gauge_draw(gfx, cfg, now, opacity);
}

/** @brief Preserve the eye_scene_gauge test/internal entry point. */
void eye_scene_gauge(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	face_defs::gauge_face::local_eye_scene_gauge(gfx, cfg, layout, elapsed, t);
}

/** @brief Preserve the eye_scene_pong test/internal entry point. */
void eye_scene_pong(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	face_defs::pong_face::local_eye_scene_pong(gfx, cfg, layout, elapsed, t);
}

/** @brief Preserve the eye_equalizer_bar test/internal entry point. */
EyeBox eye_equalizer_bar(const EyeConfig &cfg, const EyeLayout &layout, uint8_t index, uint32_t elapsed) {
	return face_defs::dancing_face::local_eye_equalizer_bar(cfg, layout, index, elapsed);
}

/** @brief Preserve the eye_equalizer_ink test/internal entry point. */
uint16_t eye_equalizer_ink(uint8_t index) {
	return face_defs::dancing_face::local_eye_equalizer_ink(index);
}

/** @brief Preserve the eye_performance_equalizer test/internal entry point. */
void eye_performance_equalizer(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed, float a) {
	face_defs::dancing_face::local_eye_performance_equalizer(gfx, cfg, layout, elapsed, a);
}

/** @brief Preserve the eye_performance_rotor test/internal entry point. */
void eye_performance_rotor(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	face_defs::menacing_face::local_eye_performance_rotor(gfx, cfg, b, elapsed, a);
}

/** @brief Preserve the eye_performance_rain test/internal entry point. */
void eye_performance_rain(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	face_defs::rain_face::local_eye_performance_rain(gfx, cfg, elapsed, a);
}

/** @brief Preserve the eye_sushi_target test/internal entry point. */
EyeBox eye_sushi_target(const EyeConfig &cfg) {
	return face_defs::sushi_face::local_eye_sushi_target(cfg);
}

/** @brief Preserve the eye_sushi_topping test/internal entry point. */
void eye_sushi_topping(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &rice, float a) {
	face_defs::sushi_face::local_eye_sushi_topping(gfx, cfg, rice, a);
}

/** @brief Preserve the eye_performance_sushi test/internal entry point. */
void eye_performance_sushi(Arduino_GFX *gfx, EyeConfig cfg, const EyeLayout &layout,
		uint32_t elapsed, float a) {
	face_defs::sushi_face::local_eye_performance_sushi(gfx, cfg, layout, elapsed, a);
}

/** @brief Preserve the eye_turbo_impeller test/internal entry point. */
void eye_turbo_impeller(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx,
		int16_t cy, int16_t rx, int16_t ry, float angle, float a) {
	face_defs::energetic_face::local_eye_turbo_impeller(gfx, cfg, cx, cy, rx, ry, angle, a);
}

/** @brief Preserve the eye_turbo_target test/internal entry point. */
EyeBox eye_turbo_target(const EyeConfig &cfg, bool left) {
	return face_defs::energetic_face::local_eye_turbo_target(cfg, left);
}

/** @brief Preserve the eye_turbo_smoke test/internal entry point. */
void eye_turbo_smoke(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy,
		int16_t r, bool left, uint32_t elapsed, float a) {
	face_defs::energetic_face::local_eye_turbo_smoke(gfx, cfg, cx, cy, r, left, elapsed, a);
}

/** @brief Preserve the eye_performance_turbo test/internal entry point. */
void eye_performance_turbo(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	face_defs::energetic_face::local_eye_performance_turbo(gfx, cfg, b, elapsed, a);
}

/** @brief Preserve the eye_performance_stars test/internal entry point. */
void eye_performance_stars(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	face_defs::stargazing_face::local_eye_performance_stars(gfx, cfg, elapsed, a);
}

/** @brief Preserve the eye_stargazing_look test/internal entry point. */
float eye_stargazing_look(uint32_t elapsed) {
	return face_defs::stargazing_face::local_eye_stargazing_look(elapsed);
}

/** @brief Preserve the eye_extra_hearts test/internal entry point. */
void eye_extra_hearts(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	face_defs::adore_face::local_eye_extra_hearts(gfx, cfg, elapsed, a);
}

/** @brief Apply generic ordinary-reaction geometry with unchanged clamps. */
void eye_reaction_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed) {
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

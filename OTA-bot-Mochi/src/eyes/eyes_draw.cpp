/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * @file eyes_draw.cpp
 * @brief Bounded solid/pupilled eye rendering with correct dirty-region unions.
 */
#include "eye_shapes.h"
#include "eye_palette.h"
#include "eye_life.h"
#include "eye_reference.h"
#include "eye_reactions.h"
#include "eye_effects.h"
#include "eye_scenes.h"
#include "eye_performance_draw.h"
#include "../../pin_config.h"

/** @brief Clear the previous complete union. @param gfx Canvas. @param cfg Config. @param st State. */
static void wipe(Arduino_GFX *gfx, const EyeConfig &cfg, EyeState &st) {
	if (!st.dirtyInit) { gfx->fillScreen(cfg.bgColor); st.dirtyInit = true; }
	else { gfx->fillRect(st.prevX, st.prevY, st.prevW, st.prevH, cfg.bgColor); }
}

/** @brief Store the true min/max of both eyes, including asymmetric heights. @param st State. @param layout Eyes. */
static void store_union(EyeState &st, const EyeLayout &layout, const EyeConfig &cfg) {
	const EyeBox &a = layout.left, &b = layout.right;
	int16_t left = (a.x < b.x ? a.x : b.x) - 2;
	int16_t top = (a.y < b.y ? a.y : b.y) - 2;
	int16_t right = (a.x + a.w > b.x + b.w ? a.x + a.w : b.x + b.w) + 2;
	int16_t bottom = (a.y + a.h > b.y + b.h ? a.y + a.h : b.y + b.h) + 2;
	st.prevX = eye_limit(left, 0, cfg.screenW - 1);
	st.prevY = eye_limit(top, 0, cfg.screenH - 1);
	st.prevW = eye_limit(right, st.prevX + 1, cfg.screenW) - st.prevX;
	st.prevH = eye_limit(bottom, st.prevY + 1, cfg.screenH) - st.prevY;
}

/** @brief Expand the stored union with the petting heart box. @param st State. */
static void extend_heart(EyeState &st) {
	int16_t left = eye_limit(int16_t(st.heartX - 7), 0, LCD_WIDTH - 1);
	int16_t top = eye_limit(int16_t(st.heartY - 1), 0, LCD_HEIGHT - 1);
	int16_t right = eye_limit(int16_t(st.heartX + 8), int16_t(left + 1), int16_t(LCD_WIDTH));
	int16_t bottom = eye_limit(int16_t(st.heartY + 13), int16_t(top + 1), int16_t(LCD_HEIGHT));
	if (left < st.prevX) { st.prevW = int16_t(st.prevW + (st.prevX - left)); st.prevX = left; }
	if (top < st.prevY) { st.prevH = int16_t(st.prevH + (st.prevY - top)); st.prevY = top; }
	if (right > int16_t(st.prevX + st.prevW)) { st.prevW = int16_t(right - st.prevX); }
	if (bottom > int16_t(st.prevY + st.prevH)) { st.prevH = int16_t(bottom - st.prevY); }
}

/** @brief Draw the heart marker; box spans cx-6..cx+6 and top..top+11. @param gfx Canvas. @param color RGB565. @param cx Center X. @param top Top Y. */
static void paint_heart(Arduino_GFX *gfx, uint16_t color, int16_t cx, int16_t top) {
	gfx->fillCircle(cx - 3, top + 3, 3, color);
	gfx->fillCircle(cx + 3, top + 3, 3, color);
	gfx->fillTriangle(cx - 6, top + 4, cx + 6, top + 4, cx, top + 11, color);
}

/** @brief Paint one eye, keeping blink/symbol logic separate from pupil rendering. @param gfx Canvas. @param cfg Config. @param st State. @param box Eye. @param elapsed Mood-relative time. @param neutral Selected ink. */
static void paint(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &box, uint32_t elapsed, uint16_t neutral) {
	if (st.blinking || box.h <= 13) {
		gfx->fillRoundRect(box.x, box.y, box.w, box.h, (box.w < box.h ? box.w : box.h) / 2, cfg.fgColor);
		// Short eyes drop pupils, but they must keep their lid mask: on a small
		// panel two half-closed names would otherwise become the same bar.
		if (!st.blinking) { eye_lids(gfx, cfg, box); }
		return;
	}
	if (eye_reaction_body(gfx, cfg, st, box, elapsed, neutral) || eye_reference_body(gfx, cfg, st, box)) { return; }
	eye_body(gfx, cfg, box);
	eye_pupils(gfx, cfg, st, box);
	eye_lids(gfx, cfg, box);
}

/** @brief Apply soft opening/stretch while keeping the full silhouette on screen. @param box Eye. @param pose Motion. */
static void animate_box(EyeBox &box, const EyeLifeFrame &pose, const EyeConfig &cfg) {
	int16_t cx = box.x + box.w / 2, cy = box.y + box.h / 2;
	box.w = eye_limit(int16_t(box.w * pose.scaleX), 20, 100);
	box.h = eye_limit(int16_t(box.h * pose.scaleY * (box.left ? pose.openL : pose.openR)), 6, 138);
	box.x = eye_limit(cx - box.w / 2, 0, cfg.screenW - box.w);
	box.y = eye_limit(cy - box.h / 2, 0, cfg.screenH - box.h);
	box.radius = eye_limit(box.radius, 0, (box.w < box.h ? box.w : box.h) / 2);
}

/** @brief Draw a full-face animated scene instead of the pair. @param gfx Canvas. @param cfg Config. @param st State. @param layout Eyes. @return Scene handled. */
static bool paint_scene(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, const EyeLayout &layout) {
	if (cfg.expression != GAUGE && cfg.expression != PONG) { return false; }
	float t = eye_scene_amount(elapsed);
	if (t == 0) { return false; }
	if (cfg.expression == GAUGE) { eye_scene_gauge(gfx, cfg, layout, elapsed, t); }
	else { eye_scene_pong(gfx, cfg, layout, elapsed, t); }
	return true;
}

/** @brief Render distinct style silhouettes and mask lids after pupils to avoid floating highlights. */
void eyes_draw() {
	Arduino_GFX *gfx = eyes_gfx();
	if (!gfx) { return; }
	EyeConfig cfg = eyes_config();
	EyeState &st = eyes_state();
	EyeState drawing = st;
	const EyeLifeFrame &pose = eyes_life_frame();
	if (pose.active) {
		cfg.expression = pose.expression; drawing.blinking = false;
		drawing.saccadeX = drawing.saccadeY = drawing.floatY = 0;
		drawing.tiltX = pose.x; drawing.tiltY = pose.y;
	}
	bool scene = cfg.expression == PONG || cfg.expression == GAUGE || eye_performance(cfg.expression);
	uint32_t elapsed = eye_effect_elapsed(st, cfg.expression);
	if (scene) { drawing = EyeState{}; }
	EyeLayout layout = eye_layout(cfg, drawing);
	if (pose.active && !scene) { animate_box(layout.left, pose, cfg); animate_box(layout.right, pose, cfg); }
	if (!drawing.blinking && !scene) { eye_reaction_layout(cfg, layout, elapsed); }
	eye_performance_layout(cfg, layout, elapsed);
	if (cfg.expression == STARGAZING) {
		drawing.saccadeY = -int16_t(layout.left.h * eye_stargazing_look(elapsed) / 2);
	}
	cfg.fgColor = eye_reaction_ink(cfg, elapsed);
	wipe(gfx, cfg, st);
	EyeState effects = st;
	if (cfg.expression == GLOW || cfg.expression == HEADLIGHTS) { effects.lastMs = elapsed; }
	eye_effect_background(gfx, cfg, effects, layout);
	eye_performance_background(gfx, cfg, layout, elapsed);
	if (!paint_scene(gfx, cfg, elapsed, layout) && !eye_performance_scene(gfx, cfg, layout, elapsed)) {
		EyeConfig left = cfg, right = cfg;
		left.fgColor = eye_performance_ink(cfg, true, elapsed);
		right.fgColor = eye_performance_ink(cfg, false, elapsed);
		paint(gfx, left, drawing, layout.left, elapsed, eyes_config().fgColor);
		paint(gfx, right, drawing, layout.right, elapsed, eyes_config().fgColor);
	}
	if (st.heartUntilMs != 0) {
		paint_heart(gfx, cfg.fgColor, st.heartX, st.heartY);
	}
	store_union(st, eye_effect_bounds(cfg, layout), cfg);
	// Particles and reversible morphs travel beyond the ordinary pair's rectangle.
	if (eye_performance(cfg.expression)) { st.prevX = st.prevY = 0; st.prevW = cfg.screenW; st.prevH = cfg.screenH; }
	if (st.heartUntilMs != 0) { extend_heart(st); }
}

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
#include "eye_life.h"
#include "../../pin_config.h"

/** @brief Clear the previous complete union. @param gfx Canvas. @param cfg Config. @param st State. */
static void wipe(Arduino_GFX *gfx, const EyeConfig &cfg, EyeState &st) {
	if (!st.dirtyInit) { gfx->fillScreen(cfg.bgColor); st.dirtyInit = true; }
	else { gfx->fillRect(st.prevX, st.prevY, st.prevW, st.prevH, cfg.bgColor); }
}

/** @brief Store the true min/max of both eyes, including asymmetric heights. @param st State. @param layout Eyes. */
static void store_union(EyeState &st, const EyeLayout &layout) {
	const EyeBox &a = layout.left, &b = layout.right;
	int16_t left = (a.x < b.x ? a.x : b.x) - 2;
	int16_t top = (a.y < b.y ? a.y : b.y) - 2;
	int16_t right = (a.x + a.w > b.x + b.w ? a.x + a.w : b.x + b.w) + 2;
	int16_t bottom = (a.y + a.h > b.y + b.h ? a.y + a.h : b.y + b.h) + 2;
	st.prevX = eye_limit(left, 0, LCD_WIDTH - 1);
	st.prevY = eye_limit(top, 0, LCD_HEIGHT - 1);
	st.prevW = eye_limit(right, st.prevX + 1, LCD_WIDTH) - st.prevX;
	st.prevH = eye_limit(bottom, st.prevY + 1, LCD_HEIGHT) - st.prevY;
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

/** @brief Paint one eye, keeping blink/symbol logic separate from pupil rendering. @param gfx Canvas. @param cfg Config. @param st State. @param box Eye. */
static void paint(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &box) {
	if (st.blinking || box.h <= 13) {
		gfx->fillRoundRect(box.x, box.y, box.w, box.h, box.h / 2, cfg.fgColor);
		return;
	}
	eye_body(gfx, cfg, box);
	eye_pupils(gfx, cfg, st, box);
	eye_lids(gfx, cfg, box);
}

/** @brief Apply soft opening/stretch while keeping the full silhouette on screen. @param box Eye. @param pose Motion. */
static void animate_box(EyeBox &box, const EyeLifeFrame &pose) {
	int16_t cx = box.x + box.w / 2, cy = box.y + box.h / 2;
	box.w = eye_limit(int16_t(box.w * pose.scaleX), 20, 100);
	box.h = eye_limit(int16_t(box.h * pose.scaleY * (box.left ? pose.openL : pose.openR)), 6, 138);
	box.x = eye_limit(cx - box.w / 2, 0, LCD_WIDTH - box.w);
	box.y = eye_limit(cy - box.h / 2, 0, LCD_HEIGHT - box.h);
	box.radius = eye_limit(box.radius, 0, (box.w < box.h ? box.w : box.h) / 2);
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
	EyeLayout layout = eye_layout(cfg, drawing);
	if (pose.active) { animate_box(layout.left, pose); animate_box(layout.right, pose); }
	wipe(gfx, cfg, st);
	paint(gfx, cfg, drawing, layout.left);
	paint(gfx, cfg, drawing, layout.right);
	if (st.heartUntilMs != 0) { paint_heart(gfx, cfg.fgColor, st.heartX, st.heartY); }
	store_union(st, layout);
	if (st.heartUntilMs != 0) { extend_heart(st); }
}

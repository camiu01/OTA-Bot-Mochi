/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file eyes_draw.cpp
 * @brief Eye geometry, expression overlays and frame rendering.
 */
#include "eyes_api.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <math.h>

/**
 * @brief Draw one circular pupil.
 * @param g Display instance.
 * @param cx Center X in pixels.
 * @param cy Center Y in pixels.
 * @param pr Pupil radius in pixels.
 * @param c Fill color (RGB565).
 */
static void draw_pupil(Arduino_GFX *g, int16_t cx, int16_t cy, int16_t pr, uint16_t c) {
	g->fillCircle(cx, cy, pr, c);
}

/**
 * @brief Draw specular highlight inside a pupil.
 * @param g Display instance.
 * @param cx Pupil center X in pixels.
 * @param cy Pupil center Y in pixels.
 * @param pr Pupil radius in pixels.
 * @param c Fill color (RGB565).
 */
static void draw_highlight(Arduino_GFX *g, int16_t cx, int16_t cy, int16_t pr, uint16_t c) {
	g->fillCircle(cx - pr / 3, cy - pr / 3, pr / 3 > 1 ? pr / 3 : 1, c);
}

/**
 * @brief Draw a heart-shaped pupil for LOVE expression.
 * @param g Display instance.
 * @param cx Center X in pixels.
 * @param cy Center Y in pixels.
 * @param s Heart size in pixels.
 * @param c Fill color (RGB565).
 */
static void draw_heart(Arduino_GFX *g, int16_t cx, int16_t cy, int16_t s, uint16_t c) {
	g->fillCircle(cx - s / 2, cy - s / 4, s / 2, c);
	g->fillCircle(cx + s / 2, cy - s / 4, s / 2, c);
	g->fillTriangle(cx - s, cy, cx + s, cy, cx, cy + s, c);
}

/**
 * @brief Draw a cross-shaped pupil for DIZZY expression.
 * @param g Display instance.
 * @param cx Center X in pixels.
 * @param cy Center Y in pixels.
 * @param s Half-size of the cross in pixels.
 * @param c Line color (RGB565).
 */
static void draw_cross(Arduino_GFX *g, int16_t cx, int16_t cy, int16_t s, uint16_t c) {
	for (int8_t o = -1; o <= 1; o++) {
		g->drawLine(cx - s + o, cy - s, cx + s + o, cy + s, c);
		g->drawLine(cx - s + o, cy + s, cx + s + o, cy - s, c);
	}
}

/**
 * @brief Draw one Cozmo eye body as a rounded rectangle.
 * @param g Display instance.
 * @param x Left edge in pixels.
 * @param y Top edge in pixels.
 * @param w Eye width in pixels.
 * @param h Eye height in pixels.
 * @param r Corner radius in pixels.
 * @param c Fill color (RGB565).
 */
static void draw_cozmo_eye(Arduino_GFX *g, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t c) {
	if (w < 8) {
		w = 8;
	}
	if (h < 8) {
		h = 8;
	}
	g->fillRoundRect(x, y, w, h, r, c);
}

/**
 * @brief Draw one TRAPEZ eye with corner-lifted high-left angle.
 * @param g Display instance.
 * @param x Left edge in pixels.
 * @param y Top edge in pixels.
 * @param w Eye width in pixels.
 * @param h Eye height in pixels.
 * @param r Corner radius in pixels.
 * @param slant Corner-lift in px, pulls high-left corner up.
 * @param c Fill color (RGB565).
 */
static void draw_trapez_body(Arduino_GFX *g, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, int8_t slant, uint16_t c) {
	if (h < 8) {
		h = 8;
	}
	g->fillRoundRect(x, y, w, h, r, c);
	if (slant > 0) {
		g->fillTriangle(x, y + r, (int16_t)(x + w / 2), y, x, (int16_t)(y - slant), c);
		g->fillRect(x, y, (int16_t)(w / 2), r, c);
	} else if (slant < 0) {
		g->fillTriangle(x, (int16_t)(y + h - r), (int16_t)(x + w / 2), (int16_t)(y + h), x, (int16_t)(y + h - slant), c);
	}
}

/**
 * @brief Per-eye Cozmo geometry delta.
 */
struct CozmoGeo {
	int16_t dw;	/**< Width delta in pixels. */
	int16_t dh;	/**< Height delta in pixels. */
	int16_t dy;	/**< Vertical offset in pixels. */
};

/**
 * @brief Resolve per-eye size delta for asymmetric expressions.
 * @param expr Active expression.
 * @param left True for left eye, false for right eye.
 * @param out Output geometry delta.
 */
static void cozmo_geo(Expression expr, bool left, CozmoGeo *out) {
	out->dw = 0;
	out->dh = 0;
	out->dy = 0;
	switch (expr) {
	case SKEPTIC:
		if (left) { out->dw = 10; out->dh = 12; out->dy = -6; }
		else { out->dw = -14; out->dh = -18; out->dy = 6; }
		break;
	case SUSPICIOUS:
		out->dy = left ? -10 : 10;
		if (!left) { out->dh = -8; }
		break;
	case ANNOYED:
		out->dh = left ? -14 : -22;
		out->dy = left ? 4 : 8;
		break;
	case FRUSTRATED:
		out->dw = left ? 4 : -4;
		out->dy = left ? -4 : 4;
		break;
	case WORRIED:
		out->dy = left ? -6 : -2;
		out->dh = -6;
		break;
	case SAD_DOWN:
		out->dy = 8;
		break;
	case SAD_UP:
		out->dy = -8;
		break;
	case SLEEPY_EYES:
		if (left) { out->dh = -18; out->dy = 8; }
		else { out->dh = -30; out->dy = 12; }
		break;
	default:
		break;
	}
}
/**
 * @brief Clear the dirty region from the previous frame.
 * @param g Display instance.
 * @param cfg Active eye configuration.
 * @param st Mutable animation state.
 */
static void wipe_prev(Arduino_GFX *g, EyeConfig &cfg, EyeState &st) {
	if (!st.dirtyInit) {
		g->fillScreen(cfg.bgColor);
		st.dirtyInit = true;
		return;
	}
	g->fillRect(st.prevX, st.prevY, st.prevW, st.prevH, cfg.bgColor);
}

/**
 * @brief Store the padded union of both eyes as next dirty region.
 * @param st Mutable animation state.
 * @param x Left edge of the union in pixels.
 * @param y Top edge of the union in pixels.
 * @param w Union width in pixels.
 * @param h Union height in pixels.
 */
static void store_union(EyeState &st, int16_t x, int16_t y, int16_t w, int16_t h) {
	const int16_t pad = 18;
	x -= pad;
	y -= pad;
	w += pad * 2;
	h += pad * 2;
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	st.prevX = x;
	st.prevY = y;
	st.prevW = w;
	st.prevH = h;
}

/**
 * @brief Overlay classic lid shapes.
 * @param g Display instance.
 * @param cfg Active eye configuration.
 * @param lx Left eye X in pixels.
 * @param rx Right eye X in pixels.
 * @param y Eye top edge in pixels.
 * @param w Single eye width in pixels.
 * @param h Single eye height in pixels.
 */
static void lids_classic(Arduino_GFX *g, EyeConfig &cfg, int16_t lx, int16_t rx, int16_t y, int16_t w, int16_t h) {
	int16_t lt = (int16_t)(y + cfg.lidTop);
	switch (cfg.expression) {
	case ANGRY:
		g->fillTriangle(lx, y, lx + w, y, lx + w, (int16_t)(y + 22), cfg.bgColor);
		g->fillTriangle(rx, y, rx + w, y, rx, (int16_t)(y + 22), cfg.bgColor);
		break;
	case FURIOUS:
		g->fillTriangle(lx, y, lx + w, y, lx + w, (int16_t)(y + 18), cfg.bgColor);
		g->fillTriangle(rx, y, rx + w, y, rx, (int16_t)(y + 18), cfg.bgColor);
		g->fillRect(lx, y, w, 18, cfg.bgColor);
		g->fillRect(rx, y, w, 18, cfg.bgColor);
		break;
	case SAD:
		g->fillTriangle(lx, y, lx + w, y, lx, (int16_t)(y + 22), cfg.bgColor);
		g->fillTriangle(rx, y, rx + w, y, rx + w, (int16_t)(y + 22), cfg.bgColor);
		break;
	case HAPPY:
		g->fillRect(lx, lt, w, 12, cfg.bgColor);
		g->fillRect(rx, lt, w, 12, cfg.bgColor);
		break;
	case GLEE:
		g->fillRect(lx, lt, w, 10, cfg.bgColor);
		g->fillRect(rx, lt, w, 10, cfg.bgColor);
		g->drawArc(lx + w / 2, lt + 10, 12, 8, 0, 180, cfg.bgColor);
		g->drawArc(rx + w / 2, lt + 10, 12, 8, 0, 180, cfg.bgColor);
		break;
	case FOCUSED:
		g->fillRect(lx, (int16_t)(y + h - 14), w, 14, cfg.bgColor);
		g->fillRect(rx, (int16_t)(y + h - 14), w, 14, cfg.bgColor);
		break;
	case UNIMPRESSED:
		g->fillRect(lx, lt, w, (int16_t)(h / 2), cfg.bgColor);
		g->fillRect(rx, lt, w, (int16_t)(h / 2), cfg.bgColor);
		break;
	default:
		break;
	}
}

/**
 * @brief Overlay asymmetric Cozmo lid shapes.
 * @param g Display instance.
 * @param cfg Active eye configuration.
 * @param lx Left eye X in pixels.
 * @param rx Right eye X in pixels.
 * @param y Eye top edge in pixels.
 * @param w Single eye width in pixels.
 * @param h Single eye height in pixels.
 */
static void lids_asym(Arduino_GFX *g, EyeConfig &cfg, int16_t lx, int16_t rx, int16_t y, int16_t w, int16_t h) {
	int16_t lt = (int16_t)(y + cfg.lidTop);
	switch (cfg.expression) {
	case WORRIED:
		g->fillTriangle(lx, y, lx + w / 2, y, lx + 6, (int16_t)(y + 16), cfg.bgColor);
		g->fillTriangle(rx + w / 2, y, rx + w, y, rx + w - 6, (int16_t)(y + 16), cfg.bgColor);
		break;
	case ANNOYED:
		g->fillRect(lx, lt, w, 14, cfg.bgColor);
		g->fillRect(rx, lt, w, 20, cfg.bgColor);
		break;
	case FRUSTRATED:
		g->fillTriangle(lx, y, lx + w, y, lx, (int16_t)(y + 14), cfg.bgColor);
		g->fillTriangle(rx, y, rx + w, y, rx + w, (int16_t)(y + 20), cfg.bgColor);
		break;
	case SAD_DOWN:
		g->fillTriangle(lx, y, lx + w, y, lx, (int16_t)(y + 18), cfg.bgColor);
		g->fillTriangle(rx, y, rx + w, y, rx + w, (int16_t)(y + 18), cfg.bgColor);
		break;
	case SAD_UP:
		g->fillRect(lx, (int16_t)(y + h - 16), w, 16, cfg.bgColor);
		g->fillRect(rx, (int16_t)(y + h - 16), w, 16, cfg.bgColor);
		break;
	case SUSPICIOUS:
		g->fillRect(lx, (int16_t)(lt - 8), w, 12, cfg.bgColor);
		g->fillRect(rx, (int16_t)(lt + 8), w, 16, cfg.bgColor);
		break;
	case SKEPTIC:
		g->fillRect(lx, lt, w, 8, cfg.bgColor);
		g->fillRect(rx, lt, w, 18, cfg.bgColor);
		break;
	case SQUINT:
		g->fillRect(lx, lt, w, (int16_t)(h / 3), cfg.bgColor);
		g->fillRect(rx, lt, w, (int16_t)(h / 3), cfg.bgColor);
		break;
	case SLEEPY_EYES:
		g->fillRect(lx, lt, w, (int16_t)(h * 45 / 100), cfg.bgColor);
		g->fillRect(rx, lt, w, (int16_t)(h * 60 / 100), cfg.bgColor);
		break;
	default:
		break;
	}
}

/**
 * @brief Overlay eyelid shapes for all expressions.
 * @param g Display instance.
 * @param cfg Active eye configuration.
 * @param lx Left eye X in pixels.
 * @param rx Right eye X in pixels.
 * @param y Eye top edge in pixels.
 * @param w Single eye width in pixels.
 * @param h Single eye height in pixels.
 */
static void lids_for_expression(Arduino_GFX *g, EyeConfig &cfg, int16_t lx, int16_t rx, int16_t y, int16_t w, int16_t h) {
	lids_classic(g, cfg, lx, rx, y, w, h);
	lids_asym(g, cfg, lx, rx, y, w, h);
}

/**
 * @brief Apply symmetric size deltas to base geometry.
 * @param cfg Active eye configuration.
 * @param w In-out eye width in pixels.
 * @param h In-out eye height in pixels.
 */
static void size_sym(EyeConfig &cfg, int16_t *w, int16_t *h) {
	switch (cfg.expression) {
	case SURPRISED:
		*w += 10;
		*h += 14;
		break;
	case SLEEPY:
		*h = *h * 35 / 100;
		break;
	case HAPPY:
		*h = *h * 72 / 100;
		break;
	case SQUINT:
		*h = *h * 45 / 100;
		break;
	case SCARED:
		*w -= 8;
		*h += 6;
		break;
	case AWE:
		*w += 6;
		*h += 18;
		break;
	case GLEE:
		*h += 8;
		break;
	case FOCUSED:
		*h = *h * 80 / 100;
		break;
	default:
		break;
	}
}

/**
 * @brief Resolve per-frame eye size from config, expression and blink.
 * @param cfg Active eye configuration.
 * @param st Mutable animation state.
 * @param w Output eye width in pixels.
 * @param h Output eye height in pixels.
 * @param r Output corner radius in pixels.
 */
static void resolve_size(EyeConfig &cfg, EyeState &st, int16_t *w, int16_t *h, int16_t *r) {
	*w = cfg.eyeW;
	*h = cfg.eyeH;
	*r = cfg.radius;
	size_sym(cfg, w, h);
	if (st.blinking) {
		*h = 8;
	}
}

/**
 * @brief Draw pupils or expression overlays for both eyes.
 * @param g Display instance.
 * @param cfg Active eye configuration.
 * @param st Mutable animation state.
 * @param lx Left eye X in pixels.
 * @param rx Right eye X in pixels.
 * @param ey Eye top edge in pixels.
 * @param w Single eye width in pixels.
 * @param h Single eye height in pixels.
 * @param ox Combined horizontal gaze offset.
 * @param oy Combined vertical gaze offset.
 */
static void draw_pupils(Arduino_GFX *g, EyeConfig &cfg, EyeState &st, int16_t lx, int16_t rx, int16_t ey, int16_t w, int16_t h, int16_t ox, int16_t oy) {
	int16_t pr = w < 20 ? 6 : 10;
	int16_t lpx = (int16_t)(lx + w / 2 + ox / 3);
	int16_t rpx = (int16_t)(rx + w / 2 + ox / 3);
	int16_t lpy = (int16_t)(ey + h / 2 + oy / 3);
	if (cfg.expression == LOVE) {
		draw_heart(g, lpx, lpy, 10, cfg.pupilColor);
		draw_heart(g, rpx, lpy, 10, cfg.pupilColor);
		return;
	}
	if (cfg.expression == DIZZY) {
		float wob = st.floatPhase * 2.0f;
		int16_t dx = (int16_t)(sinf(wob) * 4.0f);
		int16_t dy = (int16_t)(cosf(wob) * 4.0f);
		draw_cross(g, (int16_t)(lpx + dx), (int16_t)(lpy + dy), 8, cfg.pupilColor);
		draw_cross(g, (int16_t)(rpx - dx), (int16_t)(lpy - dy), 8, cfg.pupilColor);
		return;
	}
	if (cfg.style != NORMAL) {
		draw_pupil(g, lpx, lpy, pr, cfg.pupilColor);
		draw_pupil(g, rpx, lpy, pr, cfg.pupilColor);
		draw_highlight(g, lpx, lpy, pr, cfg.highlightColor);
		draw_highlight(g, rpx, lpy, pr, cfg.highlightColor);
		return;
	}
	draw_pupil(g, lpx, lpy, (int16_t)(pr - 2), cfg.pupilColor);
	draw_pupil(g, rpx, lpy, (int16_t)(pr - 2), cfg.pupilColor);
}

/**
 * @brief Redraw eyes from current config/state.
 */
void eyes_draw() {
	Arduino_GFX *g = eyes_gfx();
	if (g == nullptr) {
		return;
	}
	EyeConfig &cfg = eyes_config();
	EyeState &st = eyes_state();
	int16_t w = 0;
	int16_t h = 0;
	int16_t r = 0;
	resolve_size(cfg, st, &w, &h, &r);
	int16_t ox = st.saccadeX + st.tiltX;
	int16_t oy = st.saccadeY + st.tiltY + st.floatY;
	int16_t cx = LCD_WIDTH / 2;
	int16_t cy = LCD_HEIGHT / 2;
	int16_t lx = (int16_t)(cx - cfg.eyeGap / 2 - w + ox);
	int16_t rx = (int16_t)(cx + cfg.eyeGap / 2 + ox);
	int16_t ey = (int16_t)(cy - h / 2 + oy);
	wipe_prev(g, cfg, st);
	CozmoGeo gl = {0, 0, 0};
	CozmoGeo gr = {0, 0, 0};
	if (cfg.asym != 0) {
		cozmo_geo(cfg.expression, true, &gl);
		cozmo_geo(cfg.expression, false, &gr);
	}
	int16_t lw = (int16_t)(w + gl.dw);
	int16_t lh = (int16_t)(h + gl.dh);
	int16_t rw = (int16_t)(w + gr.dw);
	int16_t rh = (int16_t)(h + gr.dh);
	int16_t ly = (int16_t)(ey + gl.dy);
	int16_t ry = (int16_t)(ey + gr.dy);
	if (cfg.style == TRAPEZ) {
		draw_trapez_body(g, lx, ly, lw, lh, r, cfg.slant, cfg.fgColor);
		draw_trapez_body(g, rx, ry, rw, rh, r, cfg.slant, cfg.fgColor);
	} else {
		draw_cozmo_eye(g, lx, ly, lw, lh, r, cfg.fgColor);
		draw_cozmo_eye(g, rx, ry, rw, rh, r, cfg.fgColor);
	}
	lids_for_expression(g, cfg, lx, rx, ey, w, h);
	if (st.blinking) {
		g->fillRect(lx, ly + lh / 2 - 2, lw, 4, cfg.fgColor);
		g->fillRect(rx, ry + rh / 2 - 2, rw, 4, cfg.fgColor);
	} else {
		draw_pupils(g, cfg, st, lx, rx, ey, w, h, ox, oy);
	}
	int16_t ux = lx < rx ? lx : rx;
	int16_t uw = (lx < rx ? rx + rw - lx : lx + lw - rx);
	int16_t uy = ly < ry ? ly : ry;
	int16_t uh = (ly < ry ? ry + rh - ly : ly + lh - ry);
	store_union(st, ux, uy, uw, uh);
}

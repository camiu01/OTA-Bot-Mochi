/**
 * @file eye_shapes.cpp
 * @brief Solid Dasai eyes, squishy blobs, ovals, and mirrored in-bounds trapezoids.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eye_shapes.h"
#include <math.h>

/** @brief Draw a heart within its requested size. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void heart(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	gfx->fillCircle(x - s / 2, y - s / 4, s / 2, color);
	gfx->fillCircle(x + s / 2, y - s / 4, s / 2, color);
	gfx->fillTriangle(x - s, y, x + s, y, x, y + s, color);
}

/** @brief Draw a full-eye cross without internal pupils. @param gfx Canvas. @param b Bounds. @param color Fill. */
static void cross(Arduino_GFX *gfx, const EyeBox &b, uint16_t color) {
	for (int8_t offset = -3; offset <= 3; ++offset) {
		gfx->drawLine(b.x + 4 + offset, b.y + 4, b.x + b.w - 5 + offset, b.y + b.h - 5, color);
		gfx->drawLine(b.x + b.w - 5 + offset, b.y + 4, b.x + 4 + offset, b.y + b.h - 5, color);
	}
}

/** @brief Draw inward-rising mirrored quadrilaterals, never adding spikes. @param gfx Canvas. @param cfg Config. @param b Bounds. */
static void trapezoid(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t slant = eye_limit(cfg.slant, -b.h / 4, b.h / 4);
	int16_t amount = slant < 0 ? -slant : slant;
	bool rises_right = b.left == (slant >= 0);
	int16_t top_left = b.y + (rises_right ? amount : 0);
	int16_t top_right = b.y + (rises_right ? 0 : amount);
	int16_t inset = b.w / 10;
	int16_t right = b.x + b.w - 1, bottom = b.y + b.h - 1;
	gfx->fillTriangle(b.x, top_left, right, top_right, right - inset, bottom, cfg.fgColor);
	gfx->fillTriangle(b.x, top_left, right - inset, bottom, b.x + inset, bottom, cfg.fgColor);
}

/** @brief Draw distinct bodies; solid styles use whole-eye hearts/crosses. @param gfx Canvas. @param cfg Config. @param b Eye. */
void eye_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	bool solid = cfg.style == MOCHI || cfg.style == TRAPEZ;
	if (solid && cfg.expression == LOVE) {
		int16_t size = (b.w < b.h ? b.w : b.h) / 2 - 2;
		heart(gfx, b.x + b.w / 2, b.y + b.h / 2, size, cfg.fgColor);
	} else if (solid && cfg.expression == DIZZY) {
		cross(gfx, b, cfg.fgColor);
	} else if (cfg.style == NORMAL) {
		gfx->fillEllipse(b.x + b.w / 2, b.y + b.h / 2, b.w / 2 - 1, b.h / 2 - 1, cfg.fgColor);
	} else if (cfg.style == TRAPEZ) {
		trapezoid(gfx, cfg, b);
	} else {
		gfx->fillRoundRect(b.x, b.y, b.w, b.h, b.radius, cfg.fgColor);
	}
}

/** @brief Draw contained pupils for the two non-solid styles. @param gfx Canvas. @param cfg Config. @param st State. @param b Eye. */
void eye_pupils(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b) {
	if (cfg.style == MOCHI || cfg.style == TRAPEZ) { return; }
	int16_t radius = eye_limit((b.w < b.h ? b.w : b.h) / 5, 2, 9);
	int16_t dx = eye_limit((st.saccadeX + st.tiltX) / 3, -b.w / 6, b.w / 6);
	int16_t dy = eye_limit((st.saccadeY + st.tiltY) / 3, -b.h / 6, b.h / 6);
	int16_t x = b.x + b.w / 2 + dx, y = b.y + b.h / 2 + dy;
	if (cfg.expression == LOVE) {
		heart(gfx, x, y, radius, cfg.pupilColor);
	} else if (cfg.expression == DIZZY) {
		EyeBox symbol = {int16_t(x - radius - 4), int16_t(y - radius - 4),
			int16_t(2 * radius + 9), int16_t(2 * radius + 9), 0, b.left};
		cross(gfx, symbol, cfg.pupilColor);
	} else {
		gfx->fillCircle(x, y, radius, cfg.pupilColor);
		if (cfg.style == BLOUB) {
			gfx->fillCircle(x - radius / 3, y - radius / 3, radius / 3, cfg.highlightColor);
		}
	}
}

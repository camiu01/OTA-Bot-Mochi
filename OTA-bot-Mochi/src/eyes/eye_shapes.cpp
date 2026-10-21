/**
 * @file eye_shapes.cpp
 * @brief Solid Dasai eyes, squishy blobs, ovals, mirrored rounded trapezoids, and expression glyphs.
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

/**
 * @brief Draw a filled five-pointed star centered in its bounding size.
 * @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill.
 */
static void star(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t px[10], py[10];
	for (int8_t i = 0; i < 10; ++i) {
		float angle = -1.5707963f + i * 0.62831853f;
		float radius = (i & 1) ? 0.42f * s : float(s);
		px[i] = int16_t(x + cosf(angle) * radius);
		py[i] = int16_t(y + sinf(angle) * radius);
	}
	for (int8_t i = 0; i < 10; ++i) {
		int8_t next = int8_t((i + 1) % 10);
		gfx->fillTriangle(x, y, px[i], py[i], px[next], py[next], color);
	}
}

/** @brief Draw a bounded Archimedean spiral. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void spiral(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t prev_x = x, prev_y = y;
	for (int8_t i = 1; i <= 24; ++i) {
		float t = i * 0.4f;
		float radius = t * s / 9.6f;
		int16_t px = int16_t(x + cosf(t) * radius);
		int16_t py = int16_t(y + sinf(t) * radius);
		gfx->drawLine(prev_x, prev_y, px, py, color);
		prev_x = px; prev_y = py;
	}
}

/** @brief Draw a small teardrop that stays within its caller's bounds. @param gfx Canvas. @param x Center. @param y Top. @param s Size. @param color Fill. */
static void tear(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { s = 2; }
	gfx->fillCircle(x, y, s, color);
	gfx->fillTriangle(x - s, y, x + s, y, x, y + 2 * s, color);
}

/** @brief Vertical offset that keeps a teardrop inside an eye. @param b Eye. @return Top of the tear. */
static int16_t tear_y(const EyeBox &b) { return b.y + b.h * 2 / 3; }

/**
 * @brief Trace a circular outline with straight segments.
 *
 * Stroking instead of subtracting a second disc keeps the glyph usable as either an
 * engraved mark on a solid eye or a coloured pupil without needing an extra palette entry.
 *
 * @param gfx Canvas. @param x Center. @param y Center. @param s Radius. @param color Stroke.
 */
static void annulus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	for (int8_t ring = 0; ring < 2 && s - ring >= 2; ++ring) {
		int16_t px = int16_t(x + s - ring), py = y;
		for (int8_t i = 1; i <= 16; ++i) {
			float angle = i * 0.392699082f;
			int16_t nx = int16_t(x + (s - ring) * cosf(angle));
			int16_t ny = int16_t(y + (s - ring) * sinf(angle));
			gfx->drawLine(px, py, nx, ny, color);
			px = nx; py = ny;
		}
	}
}

/** @brief Draw a halo ring sitting in the upper half of its box. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void halo(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	annulus(gfx, x, int16_t(y - s / 4), int16_t(s * 3 / 4), color);
}

/** @brief Draw two devil horns in the upper half of its box. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void horns(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t base = int16_t(y - s * 3 / 10);
	int16_t apex = int16_t(y - s * 9 / 10);
	int16_t outer = int16_t(x - s * 85 / 100);
	int16_t inner = int16_t(x - s / 4);
	int16_t tip = int16_t(x - s / 3);
	gfx->fillTriangle(outer, base, inner, base, tip, apex, color);
	gfx->fillTriangle(int16_t(2 * x - outer), base, int16_t(2 * x - inner), base,
		int16_t(2 * x - tip), apex, color);
}

/** @brief Draw a teardrop flame. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void flame(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t wing = int16_t(s * 3 / 4);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - wing), int16_t(y + s / 2),
		int16_t(x + wing), int16_t(y + s / 2), color);
	gfx->fillCircle(x, int16_t(y + s / 3), int16_t(s / 2), color);
}

/** @brief Draw a crescent by restoring part of an engraved disc. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. @param cut Restore color. */
static void crescent(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	if (s < 3) { return; }
	gfx->fillCircle(x, y, s, color);
	gfx->fillEllipse(int16_t(x + s / 4), y, int16_t(s * 3 / 4), int16_t(s * 3 / 4), cut);
}

/** @brief Draw a four-point sparkle. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void sparkle(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t thin = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - thin), y, int16_t(x + thin), y, color);
	gfx->fillTriangle(x, int16_t(y + s), int16_t(x - thin), y, int16_t(x + thin), y, color);
	gfx->fillTriangle(int16_t(x - s), y, x, int16_t(y - thin), x, int16_t(y + thin), color);
	gfx->fillTriangle(int16_t(x + s), y, x, int16_t(y - thin), x, int16_t(y + thin), color);
}

/** @brief Draw a zig-zag lightning bolt. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void bolt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t wing = int16_t(s * 2 / 3);
	gfx->fillTriangle(int16_t(x - wing / 2), int16_t(y - s), int16_t(x + wing),
		int16_t(y - s / 5), int16_t(x - wing / 2), int16_t(y + s / 5), color);
	gfx->fillTriangle(int16_t(x + wing / 2), int16_t(y + s), int16_t(x - wing),
		int16_t(y + s / 5), int16_t(x + wing / 2), int16_t(y - s / 5), color);
}

/** @brief Draw a rhombus. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void rhombus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t wing = int16_t(s * 2 / 3);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - wing), y, int16_t(x + wing), y, color);
	gfx->fillTriangle(x, int16_t(y + s), int16_t(x - wing), y, int16_t(x + wing), y, color);
}

/** @brief Draw an annulus crossed by a full-width and full-height line. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void crosshair(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	annulus(gfx, x, y, s, color);
	if (s < 3) { return; }
	gfx->drawLine(int16_t(x - s), y, int16_t(x + s), y, color);
	gfx->drawLine(x, int16_t(y - s), x, int16_t(y + s), color);
}

/** @brief Draw a currency mark: an ellipse crossed by a vertical stroke. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void money(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	annulus(gfx, x, y, int16_t(s * 3 / 4), color);
	if (s < 3) { return; }
	gfx->drawLine(x, int16_t(y - s), x, int16_t(y + s), color);
}

/** @brief Draw horizontal displaced glitch bars. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void glitch_glyph(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t h = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillRect(int16_t(x - s), int16_t(y - s * 2 / 3), int16_t(s * 4 / 3), h, color);
	gfx->fillRect(int16_t(x - s / 3), int16_t(y - h / 2), int16_t(s * 4 / 3), h, color);
	gfx->fillRect(int16_t(x - s * 2 / 3), int16_t(y + s / 2), int16_t(s * 4 / 3), h, color);
}

/** @brief Draw an hourglass glyph. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void hourglass(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s * 3 / 4);
	gfx->fillTriangle(int16_t(x - w), int16_t(y - s), int16_t(x + w), int16_t(y - s), x, y, color);
	gfx->fillTriangle(int16_t(x - w), int16_t(y + s), int16_t(x + w), int16_t(y + s), x, y, color);
}

/** @brief Draw a battery outline with a single low-charge bar. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void low_batt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t w = int16_t(s * 6 / 5), h = int16_t(s * 3 / 4);
	gfx->drawRect(int16_t(x - w / 2), int16_t(y - h / 2), w, h, color);
	gfx->fillRect(int16_t(x + w / 2), int16_t(y - h / 4), 2, int16_t(h / 2), color);
	gfx->fillRect(int16_t(x - w / 2 + 2), int16_t(y - h / 2 + 2), int16_t(w / 4), int16_t(h - 4), color);
}

/** @brief Draw a beamed eighth note. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void music_note(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t r = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillCircle(int16_t(x - r / 2), int16_t(y + s / 2), r, color);
	gfx->fillRect(int16_t(x + r / 2), int16_t(y - s * 2 / 3), 2, int16_t(s + r / 2), color);
	gfx->fillRect(int16_t(x + r / 2), int16_t(y - s * 2 / 3), int16_t(s / 2), int16_t(r / 2 + 1), color);
}

/** @brief Draw an IEC power standby symbol. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void power_sym(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	annulus(gfx, x, int16_t(y + s / 6), int16_t(s * 3 / 4), color);
	gfx->fillRect(int16_t(x - 1), int16_t(y - s), 3, int16_t(s), color);
}

/** @brief Draw a bold exclamation mark. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Fill. */
static void exclaim(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillRoundRect(int16_t(x - w / 2), int16_t(y - s), w, int16_t(s * 5 / 4), int16_t(w / 2), color);
	gfx->fillCircle(x, int16_t(y + s * 3 / 4), int16_t(w / 2 + 1), color);
}

/** @brief Draw a stylized question mark. @param gfx Canvas. @param x Center. @param y Center. @param s Half-size. @param color Stroke. */
static void question(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t r = int16_t(s / 2);
	annulus(gfx, x, int16_t(y - s / 3), r, color);
	gfx->fillRect(int16_t(x - 1), int16_t(y + r / 2), 2, int16_t(s / 3), color);
	gfx->fillCircle(x, int16_t(y + s * 4 / 5), int16_t(s / 5 ? s / 5 : 1), color);
}

/** @brief Circular inset that softens a vertex, tangent to the straight edge. @param soft Corner radius. @param d Distance from the vertex. @return Inset in pixels. */
static int16_t corner_pad(int16_t soft, int16_t d) {
	if (d < 0) { return soft; }
	if (d >= soft) { return 0; }
	int16_t back = int16_t(soft - d);
	return int16_t(soft - sqrtf(float(soft * soft - back * back)));
}

/**
 * @brief Span of one row of the canonical, left-eye trapezoid.
 *
 * Only the canonical quad is ever evaluated; the right eye mirrors it afterwards, which
 * keeps both eyes pixel-identical under the horizontal reflection instead of relying on
 * two independently rounded integer expressions.
 *
 * @param span Row width in pixels. @param last Last row index. @param inset Bottom taper. @param top_left Left vertex row. @param top_right Right vertex row. @param i Row. @param left Output left edge. @param right Output right edge.
 */
static void trapezoid_row(int16_t span, int16_t last, int16_t inset, int16_t top_left,
		int16_t top_right, int16_t i, int16_t &left, int16_t &right) {
	left = 0; right = span;
	if (i > top_left && last > top_left) { left = int16_t(inset * (i - top_left) / (last - top_left)); }
	if (i > top_right && last > top_right) { right = int16_t(span - inset * (i - top_right) / (last - top_right)); }
	int16_t top_min = int16_t(top_left < top_right ? top_left : top_right);
	int16_t top_max = int16_t(top_left < top_right ? top_right : top_left);
	if (i < top_min) { left = 1; right = 0; return; }
	if (top_min == top_max || i >= top_max) { return; }
	int16_t run = int16_t(top_max - top_min);
	int16_t along = int16_t(span * (top_max == top_right ? (i - top_min) : (top_max - i)) / run);
	if (top_max == top_right) { if (along < right) { right = along; } }
	else if (along > left) { left = along; }
}

/**
 * @brief Draw the mirrored trapezoid with rounded corners rather than sharp vertices.
 *
 * Rows span the slanted quadrilateral, then both ends pull inward following a circular
 * profile near the top and bottom, so every vertex reads as a curve on the panel.
 *
 * @param gfx Canvas. @param cfg Config. @param b Bounds.
 */
static void trapezoid(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (b.h < 4 || b.w < 4) { return; }
	int16_t slant = eye_limit(cfg.slant * eye_scale(cfg) / 100, -b.h / 4, b.h / 4);
	int16_t amount = int16_t(slant < 0 ? -slant : slant);
	bool rises_right = slant >= 0;
	int16_t top_left = rises_right ? amount : 0;
	int16_t top_right = rises_right ? 0 : amount;
	int16_t top_min = int16_t(top_left < top_right ? top_left : top_right);
	int16_t inset = int16_t(b.w / 10);
	int16_t last = int16_t(b.h - 1);
	int16_t span = int16_t(b.w - 1);
	int16_t shortest = (b.w < b.h ? b.w : b.h);
	int16_t base = int16_t(shortest / 6);
	int16_t soft = eye_limit(b.radius < base ? base : b.radius, 4, int16_t(shortest / 2));
	for (int16_t i = 0; i < b.h; ++i) {
		int16_t left = 0, right = span;
		trapezoid_row(span, last, inset, top_left, top_right, i, left, right);
		if (!b.left) { int16_t old = left; left = int16_t(span - right); right = int16_t(span - old); }
		if (right < left) { continue; }
		int16_t pad = corner_pad(soft, int16_t(i - top_min));
		int16_t tail = corner_pad(soft, int16_t(last - i));
		if (tail > pad) { pad = tail; }
		left = int16_t(left + pad);
		right = int16_t(right - pad);
		if (right < left) { continue; }
		gfx->fillRect(int16_t(b.x + left), int16_t(b.y + i), int16_t(right - left + 1), 1,
			cfg.fgColor);
	}
}

/** @brief Draw the style's plain body before any expression overlay. @param gfx Canvas. @param cfg Config. @param b Eye. */
static void base_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (cfg.style == NORMAL) {
		gfx->fillEllipse(int16_t(b.x + b.w / 2), int16_t(b.y + b.h / 2),
			int16_t(b.w / 2 - 1), int16_t(b.h / 2 - 1), cfg.fgColor);
	} else if (cfg.style == TRAPEZ) {
		trapezoid(gfx, cfg, b);
	} else {
		gfx->fillRoundRect(b.x, b.y, b.w, b.h, b.radius, cfg.fgColor);
	}
}

/** @brief Expressions whose glyph replaces the whole solid eye instead of engraving it. @param expr Expression. @return True when the glyph owns the eye. */
static bool replaces_solid(Expression expr) {
	return expr == LOVE || expr == DIZZY || expr == STAR || expr == SWIRLY;
}

/**
 * @brief Draw one expression glyph, never leaving a circle of radius s around (x, y).
 *
 * Keeping every primitive inside that circle is what lets the same call serve a whole-eye
 * mark on MOCHI/TRAPEZ and a pupil-sized mark on NORMAL/BLOUB without leaving the dirty region.
 * Decorations use the configured pupil ink, or an engraving on solid styles.
 *
 * @param gfx Canvas. @param cfg Config. @param b Eye. @param x Center. @param y Center. @param s Half-size. @param color Fill. @param cut Restored color for subtractive glyphs.
 * @return True when this expression owns a glyph.
 */
static bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x,
		int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	switch (cfg.expression) {
		case LOVE: heart(gfx, x, y, s, color); return true;
		case STAR: star(gfx, x, y, s, color); return true;
		case SWIRLY: spiral(gfx, x, y, s, color); return true;
		case CRYING: tear(gfx, x, tear_y(b), 2, color); return true;
		case HALO: halo(gfx, x, y, s, color); return true;
		case DEVIL: horns(gfx, x, y, s, color); return true;
		case FIRE: flame(gfx, x, y, s, color); return true;
		case MOON: crescent(gfx, x, y, s, color, cut); return true;
		case SPARKLE: sparkle(gfx, x, y, s, color); return true;
		case BOLT: bolt(gfx, x, y, s, color); return true;
		case DIAMOND: rhombus(gfx, x, y, s, color); return true;
		case RING: annulus(gfx, x, y, s, color); return true;
		case TARGET: crosshair(gfx, x, y, s, color); return true;
		case MONEY: money(gfx, x, y, s, color); return true;
		case MUSIC: music_note(gfx, x, y, s, color); return true;
		case LOW_BATT: low_batt(gfx, x, y, s, color); return true;
		case POWER: power_sym(gfx, x, y, s, color); return true;
		case LOADING: case CALCULATING: hourglass(gfx, x, y, s, color); return true;
		case IDEA: exclaim(gfx, x, y, s, color); return true;
		case GLITCH: glitch_glyph(gfx, x, y, s, color); return true;
		case QUESTION: question(gfx, x, y, s, color); return true;
		default: return false;
	}
}

/** @brief Draw distinct bodies; solid styles engrave every accessory glyph. @param gfx Canvas. @param cfg Config. @param b Eye. */
void eye_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	bool solid = cfg.style == MOCHI || cfg.style == TRAPEZ;
	int16_t cx = int16_t(b.x + b.w / 2), cy = int16_t(b.y + b.h / 2);
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 2 - 2, 3, 60);
	if ((solid && replaces_solid(cfg.expression)) || cfg.expression == LOVE || cfg.expression == STAR) {
		if (cfg.expression == DIZZY) { cross(gfx, b, cfg.fgColor); }
		else { glyph(gfx, cfg, b, cx, cy, s, cfg.fgColor, cfg.bgColor); }
		return;
	}
	base_body(gfx, cfg, b);
	if (solid) {
		glyph(gfx, cfg, b, cx, cy, int16_t(s - 1), cfg.bgColor, cfg.fgColor);
	}
}

/** @brief Draw contained pupils and symbols for the two non-solid styles. @param gfx Canvas. @param cfg Config. @param st State. @param b Eye. */
void eye_pupils(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b) {
	if (cfg.style == MOCHI || cfg.style == TRAPEZ || cfg.expression == LOVE || cfg.expression == STAR) { return; }
	int16_t radius = eye_limit((b.w < b.h ? b.w : b.h) / 5, 2, 9);
	int16_t dx = eye_limit((st.saccadeX + st.tiltX) / 3, -b.w / 6, b.w / 6);
	int16_t dy = eye_limit((st.saccadeY + st.tiltY) / 3, -b.h / 6, b.h / 6);
	if (cfg.expression == SIDE_EYE) { dx = eye_limit(b.w / 5, 2, b.w / 5); }
	int16_t x = int16_t(b.x + b.w / 2 + dx), y = int16_t(b.y + b.h / 2 + dy);
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 4, 3, 16);
	if (cfg.expression == DIZZY) {
		EyeBox symbol = {int16_t(x - radius - 4), int16_t(y - radius - 4),
			int16_t(2 * radius + 9), int16_t(2 * radius + 9), 0, b.left};
		cross(gfx, symbol, cfg.pupilColor);
	} else if (glyph(gfx, cfg, b, x, y, s, cfg.pupilColor, cfg.bgColor)) {
		return;
	} else {
		gfx->fillCircle(x, y, radius, cfg.pupilColor);
		if (cfg.style == BLOUB) {
			gfx->fillCircle(x - radius / 3, y - radius / 3, radius / 3, cfg.highlightColor);
		}
	}
}

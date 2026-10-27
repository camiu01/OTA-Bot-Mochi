#include "face_support.h"
using namespace face_primitives;

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
	const FacePreset *p = face_preset(expr);
	return p && p->replacesSolid;
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
	const FacePreset *p = face_preset(cfg.expression);
	return p && p->glyph && p->glyph(gfx, cfg, b, x, y, s, color, cut);
}

/** @brief Draw distinct bodies; solid styles engrave every accessory glyph. @param gfx Canvas. @param cfg Config. @param b Eye. */
void eye_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	bool solid = cfg.style == MOCHI || cfg.style == TRAPEZ;
	int16_t cx = int16_t(b.x + b.w / 2), cy = int16_t(b.y + b.h / 2);
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 2 - 2, 3, 60);
	const FacePreset *p = face_preset(cfg.expression);
	if ((solid && replaces_solid(cfg.expression)) || (p && p->wholeEye)) {
		if ((p && p->cross)) { cross(gfx, b, cfg.fgColor); }
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
	const FacePreset *p = face_preset(cfg.expression);
	if (cfg.style == MOCHI || cfg.style == TRAPEZ || (p && p->wholeEye)) { return; }
	int16_t radius = eye_limit((b.w < b.h ? b.w : b.h) / 5, 2, 9);
	int16_t dx = eye_limit((st.saccadeX + st.tiltX + (p ? int16_t(p->gazeX * 20) : 0)) / 3, -b.w / 6, b.w / 6);
	int16_t dy = eye_limit((st.saccadeY + st.tiltY + (p ? int16_t(p->gazeY * 18) : 0)) / 3, -b.h / 6, b.h / 6);
	if (p && p->sideGaze) { dx = eye_limit(b.w / 5, 2, b.w / 5); }
	int16_t x = int16_t(b.x + b.w / 2 + dx), y = int16_t(b.y + b.h / 2 + dy);
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 4, 3, 16);
	if ((p && p->cross)) {
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

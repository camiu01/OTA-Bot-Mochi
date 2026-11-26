/** @file ui_font.h @brief Shared bitmap and proportional typography for the AMOLED interface. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "ui_theme.h"
#include "font_choice.h"
#include "../../pin_config.h"

namespace ui_font {
constexpr bool Supported = MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448;
inline UiFont selected = UiFont::Default;
inline void select(UiFont font) { selected = font; }
inline bool active() { return Supported && selected > UiFont::Default && selected < UiFont::Count; }
inline UiFont next(UiFont font) { return UiFont((uint8_t(font) + 1) % uint8_t(UiFont::Count)); }
inline const char *name(UiFont font) {
	if (font == UiFont::Inter) { return "Inter"; }
	if (font == UiFont::DmSans) { return "DM Sans"; }
	return "Default";
}

#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
namespace detail {
struct Glyph { uint16_t offset; uint8_t width, height, advance; int8_t x, y; };
#include "fonts/mochi_sans_medium.h"
#include "fonts/mochi_sans_semibold.h"
#include "fonts/mochi_dm_sans_medium.h"
#include "fonts/mochi_dm_sans_semibold.h"

inline unsigned index(char code) { return code >= 32 && code <= 126 ? code - 32 : '?' - 32; }
inline bool medium(uint8_t size, bool raw) { return size <= (raw ? 2 : 1); }
struct Face { const Glyph *glyphs; const uint8_t *bitmap; };
inline Face face(uint8_t size, bool raw) {
	const bool body = medium(size, raw);
	if (selected == UiFont::DmSans) {
		return body ? Face{dm_medium_glyphs, dm_medium_bitmap} : Face{dm_semibold_glyphs, dm_semibold_bitmap};
	}
	return body ? Face{medium_glyphs, medium_bitmap} : Face{semibold_glyphs, semibold_bitmap};
}
inline int16_t scale(uint8_t size, bool raw) {
	if (raw) { return (8 * size * 256) / (medium(size, raw) ? 20 : 24); }
	return size < 3 ? 256 : (ui_text_size(size) * 256) / 3;
}
inline int16_t scaled(int value, int16_t scale) { return int16_t(value * scale / 256); }

inline Arduino_GFX *bound_gfx = nullptr;
inline uint16_t *bound_pixels = nullptr;
inline bool bound_flip = false;

/** @brief Read a single two-bit coverage sample in a glyph. */
inline uint8_t coverage(const uint8_t *bitmap, unsigned point) {
	return (bitmap[point / 4] >> (6 - (point % 4) * 2)) & 3;
}

	/** @brief Paint scaled glyph spans, blending their edges into the live canvas when available. */
struct Painter {
	Arduino_GFX *gfx;
	int16_t top, left, right, factor, baseline;
	uint16_t ink, background;

	void span(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t alpha) const {
		if (!alpha || w <= 0 || h <= 0) { return; }
		if (alpha == 3 || !bound_pixels || bound_gfx != gfx) {
			gfx->fillRect(x, y, w, h, alpha == 3 ? ink : ui_mix(background, ink, alpha * 85));
			return;
		}
		for (int16_t dy = 0; dy < h; ++dy) {
			for (int16_t dx = 0; dx < w; ++dx) {
				const int pos = (y + dy) * LCD_WIDTH + x + dx;
				auto &pixel = bound_pixels[bound_flip ? LCD_WIDTH * LCD_HEIGHT - 1 - pos : pos];
				pixel = ui_mix(pixel, ink, alpha * 85);
			}
		}
	}

	void glyph(const Glyph &glyph, const uint8_t *bitmap, int16_t cursor) const {
		for (uint8_t y = 0; y < glyph.height; ++y) {
			const int16_t row_y = top + scaled(baseline + glyph.y + y, factor);
			const int16_t y0 = row_y < 0 ? 0 : row_y;
			const int16_t row_end = top + scaled(baseline + glyph.y + y + 1, factor);
			const int16_t y1 = row_end > gfx->height() ? gfx->height() : row_end;
			if (y1 <= y0) { continue; }
			const unsigned row = unsigned(y) * glyph.width;
			for (uint8_t start = 0; start < glyph.width;) {
				const uint8_t alpha = coverage(bitmap + glyph.offset, row + start);
				uint8_t end = start + 1;
				while (end < glyph.width && coverage(bitmap + glyph.offset, row + end) == alpha) { ++end; }
				const int16_t x0 = cursor + scaled(glyph.x + start, factor);
				const int16_t x1 = cursor + scaled(glyph.x + end, factor);
				const int16_t begin = x0 < left ? left : x0, finish = x1 > right ? right : x1;
				span(begin, y0, finish - begin, y1 - y0, alpha);
				start = end;
			}
		}
	}
};
}
#endif

/** @brief Bind the RGB565 surface for correct antialiasing over any background. */
inline void bind(Arduino_GFX *gfx, uint16_t *pixels, bool flipped = false) {
#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
	detail::bound_gfx = gfx;
	detail::bound_pixels = pixels;
	detail::bound_flip = flipped;
#else
	(void)gfx; (void)pixels; (void)flipped;
#endif
}

/** @brief Return the selected face's advance for one sanitized character. */
inline int16_t advance(char code, uint8_t size, bool raw = false, uint16_t zoom = 256) {
#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
	if (active()) {
		const auto &g = detail::face(size, raw).glyphs[detail::index(code)];
		const int16_t scaled = detail::scaled(g.advance, detail::scale(size, raw) * zoom / 256);
		return scaled ? scaled : 1;
	}
#else
	(void)code;
#endif
	return 6 * (raw ? size : ui_text_size(size)) * zoom / 256;
}

/** @brief Measure sanitized display text in the chosen face. */
inline int16_t width(const String &ascii, uint8_t size, bool raw = false, uint16_t zoom = 256) {
	int32_t pixels = 0;
	for (unsigned i = 0; i < ascii.length() && pixels < INT16_MAX; ++i) { pixels += advance(ascii[i], size, raw, zoom); }
	return pixels < INT16_MAX ? int16_t(pixels) : INT16_MAX;
}

/** @brief Find the end of a line without splitting or dropping a glyph. */
inline unsigned line_end(const String &ascii, unsigned start, int16_t budget, uint8_t size,
	bool raw = false, uint16_t zoom = 256) {
	unsigned end = start;
	for (; end < ascii.length(); ++end) {
		const int16_t next = advance(ascii[end], size, raw, zoom);
		if (next > budget) { break; }
		budget -= next;
	}
	return end;
}

/** @brief Ellipsize a sanitized proportional label to an exact pixel budget. */
inline String fit(const String &ascii, int16_t max_width, uint8_t size, bool raw = false, uint16_t zoom = 256) {
	if (max_width <= 0) { return String(); }
	if (width(ascii, size, raw, zoom) <= max_width) { return ascii; }
	String dots = "...";
	while (dots.length() && width(dots, size, raw, zoom) > max_width) { dots = dots.substring(0, dots.length() - 1); }
	const unsigned end = line_end(ascii, 0, max_width - width(dots, size, raw, zoom), size, raw, zoom);
	return ascii.substring(0, end) + dots;
}

/** @brief Height of the text box around its baseline. */
inline int16_t height(uint8_t size, bool raw = false) {
#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
	if (active()) { return detail::scaled(detail::medium(size, raw) ? 20 : 24, detail::scale(size, raw)); }
#endif
	return 8 * (raw ? size : ui_text_size(size));
}

/** @brief Render the selected proportional face, or let the caller draw the original font. */
inline bool draw(Arduino_GFX *gfx, const String &text, int16_t x, int16_t top,
	int16_t units, uint8_t size, uint16_t ink, uint16_t background = UI_BG, bool raw = false, uint16_t zoom = 256) {
#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
	if (active()) {
		if (!size || units <= 0) { return true; }
		const auto face = detail::face(size, raw);
		const bool medium = detail::medium(size, raw);
		String ascii = ui_ascii(text);
		ascii.replace('\n', ' ');
		const String clipped = fit(ascii, units, size, raw, zoom);
		const int16_t right = x + units < gfx->width() ? x + units : gfx->width();
		const int16_t factor = detail::scale(size, raw) * zoom / 256;
		const detail::Painter painter{gfx, top, int16_t(x < 0 ? 0 : x), right, factor,
			int16_t(medium ? 15 : 18), ink, background};
		int16_t cursor = x;
		for (unsigned i = 0; i < clipped.length(); ++i) {
			painter.glyph(face.glyphs[detail::index(clipped[i])], face.bitmap, cursor);
			cursor += advance(clipped[i], size, raw, zoom);
		}
		return true;
	}
#else
	(void)gfx; (void)text; (void)x; (void)top; (void)units; (void)size; (void)ink; (void)background; (void)raw; (void)zoom;
#endif
	return false;
}

/** @brief Draw a standard label with a known fallback background. */
inline void label(Arduino_GFX *gfx, const String &text, int16_t x, int16_t top,
	int16_t units, uint8_t size, uint16_t ink, uint16_t background) {
	if (!draw(gfx, text, x, top, units, size, ink, background)) { ui_label(gfx, text, x, top, units, size, ink); }
}
}

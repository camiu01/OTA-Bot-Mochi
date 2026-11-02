/**
 * @file moon.h
 * @brief moon definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_ambient_backgrounds.h"

namespace face_defs { namespace moon_face {
using namespace face_primitives;

/** @brief Large golden crescent with a clean opening and a configured outline. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	crescent(gfx, x, y, s, color, cut);
	if (s > 5) { crescent(gfx, x, y, int16_t(s - 2), eye_rgb(255, 228, 144), cut); }
	return true;
}

/** @brief Fade the lunar sky with the same reversible cycle as the crescent eyes. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	EyeState faded = st;
	faded.backdropOpacity *= eye_performance_amount(st.lastMs);
	face_backgrounds::stars(gfx, cfg, faded, layout);
}

/** @brief Immutable moon face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "moon";
	p.normalCycle = true;
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.wholeEye = p.replacesSolid = true;
	p.background = background;
	p.bounds = face_backgrounds::full_bounds;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid moon face parameters");
} }

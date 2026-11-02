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

/** @brief Rasterize the lit hemisphere with an elliptical lunar terminator. */
inline void phase_disc(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t radius,
		float phase, uint16_t color) {
	if (radius < 1 || phase <= 0 || phase >= 1) { return; }
	float terminator = cosf(phase * 6.2831853f);
	for (int16_t row = -radius; row <= radius; ++row) {
		int16_t half = int16_t(sqrtf(float(radius * radius - row * row)));
		int16_t edge = int16_t(lroundf(half * terminator));
		int16_t left = phase < 0.5f ? edge : int16_t(-half);
		int16_t right = phase < 0.5f ? half : int16_t(-edge);
		if (right > left) { gfx->fillRect(x + left, y + row, right - left + 1, 1, color); }
	}
}

/** @brief Continuously cycle lunar phases, starting with a visible waxing crescent. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st,
		const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral;
	float phase = (st.lastMs % 6000) / 6000.0f + 0.125f;
	if (phase >= 1) { phase -= 1; }
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 2 - 2, 3, 60);
	int16_t x = b.x + b.w / 2, y = b.y + b.h / 2;
	phase_disc(gfx, x, y, s, phase, cfg.fgColor);
	if (s > 5) { phase_disc(gfx, x, y, int16_t(s - 2), phase, eye_rgb(255, 228, 144)); }
	return true;
}

/** @brief Keep the lunar sky visible throughout the phase cycle. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	face_backgrounds::stars(gfx, cfg, st, layout);
}

/** @brief Use the renderer's mood-relative clock, including the first selected frame. */
inline void drawing(EyeState &st, const EyeLayout &, uint32_t elapsed) {
	st.lastMs = elapsed;
}

/** @brief Immutable moon face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "moon";
	p.keepOpen = true;
	p.drawing = drawing;
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.wholeEye = p.replacesSolid = true;
	p.background = background;
	p.bounds = face_backgrounds::full_bounds;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid moon face parameters");
} }

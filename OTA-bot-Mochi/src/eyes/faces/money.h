/**
 * @file money.h
 * @brief money definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace money_face {
using namespace face_primitives;

/** @brief Large green dollar sign with bounded gaze movement on pupil styles. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)cut; (void)color;
	int16_t size = int16_t((b.w < b.h ? b.w : b.h) / 3);
	if (size > s) { s = size; }
	money(gfx, x, y, s, eye_rgb(48, 224, 112));
	return true;
}

/** @brief Immutable money face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "money";
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid money face parameters");
} }

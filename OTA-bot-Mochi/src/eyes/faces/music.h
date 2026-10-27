/**
 * @file music.h
 * @brief music definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace music_face {
using namespace face_primitives;

/** @brief music glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	music_note(gfx, x, y, s, color); return true;
}

/** @brief Immutable music face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "music";
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid music face parameters");
} }

/**
 * @file music.h
 * @brief music definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace music_face {
using namespace face_primitives;

/** @brief Enlarged violet eighth-note pair, bounded during pupil gaze. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)color; (void)cut;
	s = int16_t(s > (b.w < b.h ? b.w : b.h) / 3 ? s : (b.w < b.h ? b.w : b.h) / 3);
	music_note(gfx, x, y, s, eye_rgb(184, 96, 248));
	return true;
}

/** @brief Immutable music face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "music";
	p.normalCycle = true;
	p.glyph = glyph;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid music face parameters");
} }

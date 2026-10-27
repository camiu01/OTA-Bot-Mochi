/**
 * @file clock_eyes.h
 * @brief clock_eyes definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace clock_eyes_face {
using namespace face_primitives;

/** @brief clock_eyes referenceBody procedure. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral;
	eye_body(gfx, cfg, b);
	int16_t r = (b.w < b.h ? b.w : b.h) / 2 - 3;
	mini_clock_draw(gfx, b.x + b.w / 2, b.y + b.h / 2, r, st.lastMs / 1000, cfg.bgColor, cfg.bgColor);
	return true;
}

/** @brief Immutable clock_eyes face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "clock_eyes";
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid clock_eyes face parameters");
} }

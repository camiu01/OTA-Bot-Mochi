/**
 * @file clock_eyes.h
 * @brief clock_eyes definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace clock_eyes_face {
using namespace face_primitives;

/** @brief High-contrast decorative dials starting at 10:10 to keep both hands distinct. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral;
	uint32_t seconds = 36600 + st.lastMs / 1000 % 43200;
	int16_t r = (b.w < b.h ? b.w : b.h) / 2 - 3;
	if (r < 4) { return true; }
	int16_t x = b.x + b.w / 2, y = b.y + b.h / 2;
	gfx->fillCircle(x, y, r + 2, cfg.fgColor);
	gfx->fillCircle(x, y, r, cfg.bgColor);
	mini_clock_draw(gfx, x, y, r, seconds, cfg.fgColor, cfg.fgColor);
	int16_t stroke = int16_t(r / 12 + 1);
	for (int16_t offset = 0; offset < stroke; ++offset) {
		mini_clock_hand(gfx, x + offset, y, r / 2, (seconds % 43200) / 43200.0f, cfg.fgColor);
		mini_clock_hand(gfx, x, y + offset, r * 3 / 4, (seconds % 3600) / 3600.0f, cfg.fgColor);
	}
	for (int8_t i = 0; i < 4; ++i) {
		float angle = i * 1.5707963f;
		gfx->fillCircle(int16_t(x + sinf(angle) * (r - 3)), int16_t(y - cosf(angle) * (r - 3)), stroke, cfg.fgColor);
	}
	return true;
}

/** @brief Immutable clock_eyes face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "clock_eyes";
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid clock_eyes face parameters");
} }

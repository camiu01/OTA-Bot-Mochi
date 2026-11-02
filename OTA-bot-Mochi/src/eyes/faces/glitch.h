/**
 * @file glitch.h
 * @brief Bounded digital eye corruption with scanline tears and brief chromatic bursts.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace glitch_face {
/** @brief Break the eye into displaced bands while keeping every fragment inside its box. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral;
	eye_body(gfx, cfg, b);
	uint32_t cycle = st.lastMs % 2400;
	bool burst = cycle < 360 || (cycle >= 1440 && cycle < 1680);
	uint8_t phase = uint8_t(st.lastMs / (burst ? 120 : 600) + (b.left ? 0 : 3));
	int16_t line = eye_limit(int16_t(b.h / 24), 1, 6);
	if (burst) {
		face_primitives::glitch_glyph(gfx, b.x + b.w / 2, b.y + b.h / 2,
			int16_t((b.w < b.h ? b.w : b.h) / 3), cfg.bgColor);
	}
	for (uint8_t i = 0; i < 4; ++i) {
		if (!burst && i % 2 == 0) { continue; }
		int16_t y = int16_t(b.y + b.h * (i + 1) / 5 + (int16_t((phase + i) % 3) - 1) * line);
		int16_t shift = int16_t((phase + i * 3) % 5 + 1);
		int16_t edge = eye_limit(int16_t(b.w / 12 + (burst ? shift : 0)), 1, b.w / 3);
		int16_t x = int16_t(b.x + ((i + phase) % 2 ? edge : 1));
		gfx->fillRect(b.x, y, b.w, 2 * line, cfg.bgColor);
		gfx->fillRect(x, y + line, b.w - edge - 2, line, cfg.fgColor);
		if (burst || i == 1) {
			uint16_t accent = i % 2 ? eye_rgb(248, 64, 184) : eye_rgb(64, 212, 255);
			gfx->fillRect(x, y + line, edge, line, accent);
			if (burst) {
				gfx->fillRect(b.x + b.w - edge - 1, y, edge, line, eye_rgb(248, 64, 184));
				gfx->fillRect(b.x + (phase * 7 + i * 11) % (b.w - 3), y, 2, line, eye_rgb(64, 212, 255));
			}
		}
	}
	return true;
}

/** @brief Register the animated corruption without random state, delays or extra frame buffers. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "glitch";
	p.normalCycle = true;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid glitch face parameters");
} }

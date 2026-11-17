/**
 * @file portal.h
 * @brief Eyes collapse into twin rotating portals and emerge again.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace portal_face {
/** @brief A complete suction and emergence cycle with two oppositely spinning apertures. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float sink = a * (.5f - .5f * cosf((ms % 6000) * .00104719755f));
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1 - sink, 1 - sink, cfg.fgColor);
		if (a <= 0) { continue; }
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, r = b.w / 2 + 2;
		uint16_t ink = eye_mix(eye_rgb(b.left ? 160 : 64, 118, 255), cfg.bgColor, uint8_t(a * 100));
		gfx->drawCircle(cx, cy, r, ink); gfx->drawCircle(cx, cy, r + 2, ink);
		for (uint8_t i = 0; i < 5; ++i) {
			float angle = i * 1.25663706f + (b.left ? 1 : -1) * (ms % 6000) * .00314159265f;
			int16_t x = cx + int16_t(cosf(angle) * (r + 4)), y = cy + int16_t(sinf(angle) * (r + 4));
			gfx->drawLine(x, y, x + int16_t(sinf(angle) * 4), y - int16_t(cosf(angle) * 4), ink);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("portal", scene);
static_assert(face_preset_valid(preset), "Invalid Portal face");
} }

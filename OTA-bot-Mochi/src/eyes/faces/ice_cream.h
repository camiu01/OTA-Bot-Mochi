/**
 * @file ice_cream.h
 * @brief Eyes follow a strawberry ice-cream cone whose scoop melts and drips.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace ice_cream_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The scoop shrinks across the cycle while a drip grows down the cone. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float lick = (ms % 6000) / 6000.0f;
	int16_t gaze = int16_t(sinf(ms * 0.003f) * a * W / 50);
	if (a > 0) {
		uint16_t cone = tint(cfg, eye_rgb(215, 160, 90), a), scoop = tint(cfg, eye_rgb(255, 150, 190), a);
		int16_t cx = W / 2 + gaze * 2, cy = H - H / 4, s = W / 12 + 2;
		int16_t rs = int16_t(s * (1 - lick * 0.4f)) + 2;
		gfx->fillTriangle(cx - s, cy, cx + s, cy, cx, cy + s * 2, cone);
		gfx->fillCircle(cx, cy - rs / 2, rs, scoop);
		gfx->drawFastVLine(cx + rs / 2, cy, int16_t((ms / 40) % (s + 1)), scoop);
	}
	pair_body(gfx, cfg, pair, a, gaze, int16_t(a * H / 60), 1, 1 - a * 0.1f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("ice_cream", scene);
static_assert(face_preset_valid(preset), "Invalid Ice Cream face");
} }

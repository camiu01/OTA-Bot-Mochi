/**
 * @file hail.h
 * @brief Eyes wince under a red umbrella while icy hail pellets rain down on it.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace hail_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Pellets fall above the canopy; the eyes jolt on each simulated impact. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	bool hit = (ms % 700) < 120;
	if (a > 0) {
		uint16_t red = tint(cfg, eye_rgb(225, 60, 70), a), ice = tint(cfg, eye_rgb(215, 230, 240), a);
		int16_t r = W * 3 / 10, cx = W / 2, cy = eye_limit(top - H / 20, r + 3, H - 1);
		gfx->fillCircle(cx, cy, r, red);
		gfx->fillRect(cx - r - 1, cy, r * 2 + 3, r + 2, cfg.bgColor);
		gfx->drawFastVLine(cx, cy, H / 8, red);
		int16_t sky = cy - r > 8 ? cy - r : 8;
		for (int i = 0; i < 7; ++i) {
			int16_t x = int16_t((i * 41 + 11) % W);
			int16_t y = int16_t((ms / 8 + i * 29) % sky);
			gfx->fillCircle(x, y, W / 90 + 1, ice);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(a * (hit ? 2 : 0)), 1, 1 - a * (hit ? 0.55f : 0.3f), cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("hail", scene);
static_assert(face_preset_valid(preset), "Invalid Hail face");
} }

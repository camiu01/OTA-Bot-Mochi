/**
 * @file bunny.h
 * @brief Soft eyes under tall rabbit ears whose pink-lined tips twitch.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace bunny_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The right ear twitches more than the left; the pair hops slightly. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float twitch = sinf(ms * 0.012f);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	if (a > 0) {
		uint16_t fur = tint(cfg, eye_rgb(235, 230, 240), a), pink = tint(cfg, eye_rgb(255, 150, 185), a);
		int16_t rx = W / 18 + 2, ry = H / 7 + 2;
		for (int s = -1; s <= 1; s += 2) {
			int16_t cx = W / 2 + s * W / 7 + int16_t(twitch * W / 60 * (s > 0 ? 1 : 0.3f));
			gfx->fillEllipse(cx, top - ry / 2, rx, ry, fur);
			gfx->fillEllipse(cx, top - ry / 2, rx / 2, ry - 4, pink);
		}
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(fabsf(twitch) * a * 2), 1, 1 - a * 0.15f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("bunny", scene);
static_assert(face_preset_valid(preset), "Invalid Bunny face");
} }

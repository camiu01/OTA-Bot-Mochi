/**
 * @file wifi_search.h
 * @brief Searching eyes glance around under Wi-Fi arcs that light up one by one.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace wifi_search_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief One more arc lights every half second, then the signal resets. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	uint8_t bars = uint8_t((ms / 500) % 4);
	pair_body(gfx, cfg, pair, a, int16_t(sinf(ms * 0.003f) * a * W / 40), 0, 1, 1 - a * 0.35f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t on = tint(cfg, eye_rgb(90, 200, 255), a), off = tint(cfg, eye_rgb(60, 70, 90), a);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t cx = W / 2, cy = top - H / 25;
	gfx->fillCircle(cx, cy, W / 70 + 2, on);
	for (uint8_t k = 1; k <= 3; ++k) {
		int16_t r = k * (W / 22 + 2);
		for (int i = 0; i < 8; ++i) {
			float t0 = 0.785f + i * 0.196f, t1 = 0.785f + (i + 1) * 0.196f;
			gfx->drawLine(cx + int16_t(cosf(t0) * r), cy - int16_t(sinf(t0) * r),
				cx + int16_t(cosf(t1) * r), cy - int16_t(sinf(t1) * r), k <= bars ? on : off);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("wifi_search", scene);
static_assert(face_preset_valid(preset), "Invalid Wifi Search face");
} }

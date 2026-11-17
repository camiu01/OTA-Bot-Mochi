/**
 * @file yoyo.h
 * @brief Eyes ride up and down with a green yo-yo spinning on its string.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace yoyo_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The yo-yo falls and climbs on an absolute-sine path. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float p = fabsf(sinf(ms * 0.0028f)), spin = ms * 0.02f;
	int16_t x = W / 2, y = int16_t(H / 10 + p * H * 0.75f), r = W / 22 + 3;
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t((p - 0.5f) * a * H / 25);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.1f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t cord = tint(cfg, eye_rgb(220, 220, 230), a), toy = tint(cfg, eye_rgb(60, 200, 120), a);
	uint16_t hub = tint(cfg, eye_rgb(250, 250, 120), a);
	gfx->drawFastVLine(x, 0, y, cord);
	gfx->fillCircle(x, y, r, toy);
	gfx->fillCircle(x, y, r / 3 + 1, hub);
	gfx->drawLine(x + int16_t(cosf(spin) * r), y + int16_t(sinf(spin) * r), x - int16_t(cosf(spin) * r), y - int16_t(sinf(spin) * r), hub);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("yoyo", scene);
static_assert(face_preset_valid(preset), "Invalid Yoyo face");
} }

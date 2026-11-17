/**
 * @file onigiri.h
 * @brief Happy squinting eyes bounce with a hopping onigiri wrapped in nori.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace onigiri_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The rice ball and the pair share one bounce rhythm. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float hop = fabsf(sinf(ms * 0.006f));
	if (a > 0) {
		uint16_t rice = tint(cfg, eye_rgb(245, 245, 240), a), nori = tint(cfg, eye_rgb(30, 75, 50), a);
		int16_t s = W / 10 + 2, cx = W / 2, base = H - H / 12 - int16_t(hop * H / 12);
		gfx->fillTriangle(cx - s, base, cx + s, base, cx, base - s * 2, rice);
		gfx->fillRect(cx - s / 2, base - s / 2, s, s / 2, nori);
	}
	pair_body(gfx, cfg, pair, a, 0, -int16_t(a * hop * H / 30), 1 + a * 0.05f, 1 - a * 0.7f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("onigiri", scene);
static_assert(face_preset_valid(preset), "Invalid Onigiri face");
} }

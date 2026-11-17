/**
 * @file puppy.h
 * @brief Eager wide eyes between floppy brown ears, with a panting pink tongue.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace puppy_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Ears and tongue bounce with a fast tail-wag rhythm. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float wag = sinf(ms * 0.02f);
	if (a > 0) {
		uint16_t ear = tint(cfg, eye_rgb(160, 105, 60), a), tongue = tint(cfg, eye_rgb(250, 110, 130), a);
		for (auto b : {pair.left, pair.right}) {
			int16_t x = b.left ? b.x - W / 14 : b.x + b.w + W / 14;
			gfx->fillEllipse(x, b.y + b.h / 3 + int16_t(wag * 3), W / 22 + 3, H / 8, ear);
		}
		int16_t ty = pair.left.y + pair.left.h + H / 16;
		int16_t tw = W / 15, th = H / 14 + int16_t(fabsf(wag) * 4);
		gfx->fillRoundRect(W / 2 - tw / 2, ty, tw, th, (tw < th ? tw : th) / 2, tongue);
	}
	pair_body(gfx, cfg, pair, a, 0, int16_t(fabsf(wag) * a * 3), 1 + a * 0.08f, 1 + a * 0.05f, cfg.fgColor);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("puppy", scene);
static_assert(face_preset_valid(preset), "Invalid Puppy face");
} }

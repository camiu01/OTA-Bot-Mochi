/**
 * @file owl.h
 * @brief Owl eyes in brown feather rings with ear tufts and a beak; one slowly blinks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace owl_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The head tilts slowly while the left eye blinks every 2.3 seconds. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float tilt = sinf(ms * 0.002f);
	bool blink = (ms % 2300) < 250;
	if (a > 0) {
		uint16_t ring = tint(cfg, eye_rgb(180, 130, 80), a), beak = tint(cfg, eye_rgb(250, 170, 40), a);
		for (auto b : {pair.left, pair.right}) {
			gfx->fillCircle(b.x + b.w / 2, b.y + b.h / 2, (b.w > b.h ? b.w : b.h) / 2 + W / 25, ring);
		}
		int16_t cx = W / 2, cy = pair.left.y + pair.left.h / 2, s = W / 30 + 2;
		gfx->fillTriangle(cx - s, cy, cx + s, cy, cx, cy + s * 2, beak);
		int16_t top = pair.left.y - W / 25, lx = pair.left.x, rx = pair.right.x + pair.right.w;
		gfx->fillTriangle(lx, top, lx + W / 12, top, lx - W / 30, top - H / 12, ring);
		gfx->fillTriangle(rx, top, rx - W / 12, top, rx + W / 30, top - H / 12, ring);
	}
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t(tilt * a * (b.left ? 3 : -3));
		face_cinematics::body(gfx, cfg, b, a, 1 + a * 0.1f, b.left && blink ? 1 - a * 0.9f : 1 + a * 0.1f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("owl", scene);
static_assert(face_preset_valid(preset), "Invalid Owl face");
} }

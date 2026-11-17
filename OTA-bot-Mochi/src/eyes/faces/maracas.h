/**
 * @file maracas.h
 * @brief Eyes bounce in alternation while two dotted maracas shake at the sides.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace maracas_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The maracas and eyes share one fast shaking phase. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float shake = sinf(ms * 0.025f);
	if (a > 0) {
		uint16_t head = tint(cfg, eye_rgb(255, 120, 60), a), dots = tint(cfg, eye_rgb(255, 230, 90), a);
		uint16_t handle = tint(cfg, eye_rgb(150, 100, 60), a);
		int16_t r = W / 18 + 3, d = 1 + W / 160;
		for (int s = -1; s <= 1; s += 2) {
			int16_t cx = W / 2 + s * W * 3 / 8 + int16_t(s * shake * W / 40), cy = H - H / 4 - int16_t(s * shake * H / 40);
			gfx->drawLine(cx, cy + r, cx - s * W / 30, cy + r + H / 8, handle);
			gfx->fillEllipse(cx, cy, r, r + r / 3, head);
			gfx->fillCircle(cx - r / 3, cy, d, dots);
			gfx->fillCircle(cx + r / 3, cy - r / 3, d, dots);
		}
	}
	for (auto b : {pair.left, pair.right}) {
		b.y += int16_t((b.left ? shake : -shake) * a * H / 50);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * 0.35f, cfg.fgColor);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("maracas", scene);
static_assert(face_preset_valid(preset), "Invalid Maracas face");
} }

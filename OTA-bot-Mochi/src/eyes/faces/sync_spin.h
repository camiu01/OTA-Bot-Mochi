/**
 * @file sync_spin.h
 * @brief Eyes trace a small circle while two green sync arrows chase each other.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace sync_spin_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Arrow arcs and the gaze rotate on the same clock. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float t = ms * 0.004f;
	pair_body(gfx, cfg, pair, a, int16_t(cosf(t) * a * W / 50), int16_t(sinf(t) * a * H / 60), 1, 1 - a * 0.2f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t green = tint(cfg, eye_rgb(80, 220, 160), a);
	int16_t cx = W / 2, cy = pair.left.y + pair.left.h + H / 9, r = W / 14 + 3;
	for (int half = 0; half < 2; ++half) {
		float base = t + half * 3.14159f;
		for (int i = 0; i < 6; ++i) {
			float a0 = base + i * 0.4f, a1 = base + (i + 1) * 0.4f;
			gfx->drawLine(cx + int16_t(cosf(a0) * r), cy + int16_t(sinf(a0) * r),
				cx + int16_t(cosf(a1) * r), cy + int16_t(sinf(a1) * r), green);
		}
		int16_t tx = cx + int16_t(cosf(base + 2.4f) * r), ty = cy + int16_t(sinf(base + 2.4f) * r);
		gfx->fillTriangle(tx - 3, ty - 3, tx + 3, ty - 3, tx, ty + 3, green);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("sync_spin", scene);
static_assert(face_preset_valid(preset), "Invalid Sync Spin face");
} }

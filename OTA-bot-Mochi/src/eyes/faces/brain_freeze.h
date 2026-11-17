/**
 * @file brain_freeze.h
 * @brief Icy-tinted, squeezed eyes shudder under three pulsing ice crystals.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace brain_freeze_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The squeeze and crystals pulse together; a fast jitter shakes the pair. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float pain = 0.5f + 0.5f * sinf(ms * 0.009f);
	int16_t jit = int16_t(sinf(ms * 0.08f) * a * 2);
	pair_body(gfx, cfg, pair, a, jit, 0, 1 + a * 0.1f, 1 - a * (0.6f + 0.3f * pain), eye_rgb(150, 210, 255));
	if (a <= 0) { return true; }
	uint16_t ice = tint(cfg, eye_rgb(170, 230, 255), a);
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t s = W / 30 + 2;
	for (int i = 0; i < 3; ++i) {
		int16_t x = W / 2 + (i - 1) * W / 6, y = top - H / 10 - int16_t(pain * H / 30);
		gfx->drawLine(x - s, y, x, y - s, ice);
		gfx->drawLine(x, y - s, x + s, y, ice);
		gfx->drawLine(x + s, y, x, y + s, ice);
		gfx->drawLine(x, y + s, x - s, y, ice);
		gfx->drawLine(x, y - s * 2, x, y + s * 2, ice);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("brain_freeze", scene);
static_assert(face_preset_valid(preset), "Invalid Brain Freeze face");
} }

/**
 * @file dice_roll.h
 * @brief Bouncing eyes watch a white die tumble across, changing its face as it rolls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace dice_roll_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief The face advances every 160 ms using standard pip layouts. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	static constexpr uint16_t masks[7] = {0, 0x010, 0x101, 0x111, 0x145, 0x155, 0x16D};
	uint8_t side = uint8_t((ms / 160) % 6) + 1;
	float bounce = fabsf(sinf(ms * 0.006f));
	pair_body(gfx, cfg, pair, a, 0, -int16_t(bounce * a * H / 60), 1, 1 - a * 0.3f, cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t die = tint(cfg, eye_rgb(245, 240, 230), a), pip = tint(cfg, eye_rgb(200, 40, 50), a);
	int16_t s = W / 7 + 5, x = W / 2 - s / 2 + int16_t(sinf(ms * 0.004f) * W / 5), y = H - s - 4 - int16_t(bounce * H / 10);
	gfx->fillRoundRect(x, y, s, s, s / 5, die);
	for (int i = 0; i < 9; ++i) {
		if (!(masks[side] & (1 << i))) { continue; }
		gfx->fillCircle(x + s * (1 + (i % 3) * 2) / 6, y + s * (1 + (i / 3) * 2) / 6, s / 12 + 1, pip);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("dice_roll", scene);
static_assert(face_preset_valid(preset), "Invalid Dice Roll face");
} }

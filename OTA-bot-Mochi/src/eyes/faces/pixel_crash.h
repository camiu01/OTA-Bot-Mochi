/**
 * @file pixel_crash.h
 * @brief Eyes disassemble into individual pixel tiles and rebuild, without Glitch's chromatic tears.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace pixel_crash_face {
/** @brief Scatter a fixed 4x6 tile grid per eye with reproducible curved trajectories. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float scatter = a * (.35f + .325f * (1 - cosf((ms % 6000) * .00104719755f)));
	for (const auto &b : {pair.left, pair.right}) {
		if (a <= 0) { face_cinematics::body(gfx, cfg, b, 0); continue; }
		for (uint8_t row = 0; row < 6; ++row) {
			for (uint8_t col = 0; col < 4; ++col) {
				int16_t x = b.x + b.w * col / 4, y = b.y + b.h * row / 6;
				int16_t w = b.w * (col + 1) / 4 - b.w * col / 4;
				int16_t h = b.h * (row + 1) / 6 - b.h * row / 6;
				uint8_t seed = uint8_t(row * 19 + col * 37 + (b.left ? 0 : 53));
				x += int16_t((int(seed % 13) - 6) * cfg.screenW / 100.0f * scatter);
				y += int16_t((int(seed % 17) - 8) * cfg.screenH / 100.0f * scatter);
				y += int16_t(sinf(scatter * 3.14159265f + seed) * cfg.screenH / 40 * scatter);
				int16_t gap = scatter > .3f ? 1 : 0;
				if (w > gap && h > gap) { gfx->fillRect(x, y, w - gap, h - gap, cfg.fgColor); }
			}
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("pixel_crash", scene);
static_assert(face_preset_valid(preset), "Invalid Pixel Crash face");
} }

/**
 * @file hacker.h
 * @brief Scrolling eye terminals, a scanning row and an access-granted check.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace hacker_face {
/** @brief Draw fixed-size code cells inside each eye; timestamps never consume random state. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1, eye_rgb(15, 96, 56));
		if (a <= 0) { continue; }
		uint16_t code = eye_mix(eye_rgb(106, 255, 152), cfg.bgColor, uint8_t(a * 100));
		int16_t step = b.w / 6 + 1, line = b.h / 7 + 1;
		for (uint8_t row = 0; row < 5; ++row) {
			for (uint8_t col = 0; col < 4; ++col) {
				uint32_t word = (ms / 160 + row * 17 + col * 31 + (b.left ? 0 : 47)) % 7;
				if (word & 1) { gfx->fillRect(b.x + step + col * step, b.y + line + row * line, step / 2 + 1, 1, code); }
				if (word & 2) { gfx->fillRect(b.x + step + col * step, b.y + line + row * line + 1, 1, 1, code); }
			}
		}
		int16_t scan = b.y + line + (ms / 24) % (line * 5);
		gfx->drawLine(b.x + step, scan, b.x + b.w - step, scan, code);
		if (ms % 6000 >= 3300) {
			int16_t x = b.x + b.w / 2, y = b.y + b.h - line;
			gfx->drawLine(x - step, y - line / 2, x, y, code);
			gfx->drawLine(x, y, x + step, y - line, code);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("hacker", scene);
static_assert(face_preset_valid(preset), "Invalid Hacker face");
} }

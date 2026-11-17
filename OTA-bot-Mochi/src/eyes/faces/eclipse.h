/**
 * @file eclipse.h
 * @brief Two lunar discs cross glowing eyes and leave a solar corona.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace eclipse_face {
/** @brief Sweep an occulting disc across each sun while retaining a bright external corona. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1 - a, 1 - a, cfg.fgColor);
		if (a <= 0) { continue; }
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, r = b.w / 2;
		uint16_t corona = eye_mix(eye_rgb(255, 188, 72), cfg.bgColor, uint8_t(a * 100));
		int16_t offset = int16_t((1 - sinf((ms % 6000) * .000523598776f)) * r * 2);
		gfx->fillCircle(cx, cy, r, corona);
		// Scanline intersection confines the moving moon to its own sun.
		for (int16_t y = -r; y <= r; ++y) {
			int16_t half = int16_t(sqrtf(float(r * r - y * y)));
			int16_t left = offset - half;
			if (left < -half) { left = -half; }
			if (left <= half) { gfx->drawFastHLine(cx + left, cy + y, half - left + 1, cfg.bgColor); }
		}
		gfx->drawCircle(cx, cy, r + 2, corona);
		for (uint8_t i = 0; i < 8; ++i) {
			float angle = i * .785398163f;
			gfx->drawLine(cx + int16_t(cosf(angle) * (r + 4)), cy + int16_t(sinf(angle) * (r + 4)),
				cx + int16_t(cosf(angle) * (r + 7)), cy + int16_t(sinf(angle) * (r + 7)), corona);
		}
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("eclipse", scene);
static_assert(face_preset_valid(preset), "Invalid Eclipse face");
} }

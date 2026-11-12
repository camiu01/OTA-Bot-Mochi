/**
 * @file peekaboo.h
 * @brief Tiny blue roller blinds hide the eyes, then lift for a peek.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace peekaboo_face {
/** @brief Lower marked blinds over intact eyes and fully remove them at rest. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1, cfg.fgColor);
		if (a <= 0) { continue; }
		float hide = (0.5f - 0.5f * cosf(ms * 0.002855993f)) * a;
		int16_t gap = cfg.screenW / 80 + 1, top = b.y - gap;
		int16_t h = int16_t(hide * (b.h + gap)) + gap;
		uint16_t blue = eye_mix(eye_rgb(72, 135, 205), cfg.bgColor, uint8_t(a * 100));
		uint16_t seam = eye_mix(eye_rgb(145, 205, 245), cfg.bgColor, uint8_t(a * 100));
		gfx->fillRect(b.x - gap, top, b.w + gap * 2, h, blue);
		for (int16_t y = top + gap; y < top + h; y += gap * 2) {
			gfx->drawLine(b.x - gap, y, b.x + b.w + gap - 1, y, seam);
		}
		int16_t x = b.x + b.w / 2, y = top + h;
		gfx->drawLine(x, y, x, y + gap * 2, seam);
		gfx->drawCircle(x, y + gap * 3, gap, seam);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("peekaboo", scene);
static_assert(face_preset_valid(preset), "Invalid Peekaboo face");
} }

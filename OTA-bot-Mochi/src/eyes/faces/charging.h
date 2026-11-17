/**
 * @file charging.h
 * @brief Eyes open wider as a battery with a lightning bolt fills segment by segment.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace charging_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Each 1.5 seconds adds one segment; the opening follows the charge. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float level = (ms % 6000) / 6000.0f;
	pair_body(gfx, cfg, pair, a, 0, 0, 1, 1 - a * (0.6f - level * 0.7f), cfg.fgColor);
	if (a <= 0) { return true; }
	uint16_t frame = tint(cfg, eye_rgb(200, 205, 215), a), fill = tint(cfg, eye_rgb(90, 230, 110), a);
	uint16_t bolt = tint(cfg, eye_rgb(255, 230, 60), a);
	int16_t w = W / 4, h = H / 10, x = W / 2 - w / 2, y = H - h - H / 14, segs = int16_t(level * 4) + 1;
	gfx->drawRect(x, y, w, h, frame);
	gfx->fillRect(x + w, y + h / 3, 3, h / 3, frame);
	for (int16_t i = 0; i < segs && i < 4; ++i) { gfx->fillRect(x + 2 + i * (w - 4) / 4, y + 2, (w - 4) / 4 - 1, h - 4, fill); }
	gfx->fillTriangle(x + w / 2 + 2, y - 2, x + w / 2 - 4, y + h / 2 + 1, x + w / 2, y + h / 2 + 1, bolt);
	gfx->fillTriangle(x + w / 2 - 2, y + h + 2, x + w / 2 + 4, y + h / 2 - 1, x + w / 2, y + h / 2 - 1, bolt);
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("charging", scene);
static_assert(face_preset_valid(preset), "Invalid Charging face");
} }

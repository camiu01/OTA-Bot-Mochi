/**
 * @file upside_down.h
 * @brief Eyes trade vertical positions on a slow half-turn orbit.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace upside_down_face {
/** @brief Carry each eye around the pair center, then unwind on return. */
inline void layout(const EyeConfig &cfg, EyeBox &b, uint32_t, float a) {
	float angle = a * 3.14159265f;
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
	int16_t center = cfg.screenW / 2;
	int16_t x = int16_t(center + (cx - center) * cosf(angle));
	int16_t y = int16_t(cy + (cx - center) * sinf(angle));
	b.x = eye_limit(x - b.w / 2, 0, cfg.screenW - b.w);
	b.y = eye_limit(y - b.h / 2, 0, cfg.screenH - b.h);
}
/** @brief Leave a small moving orbital dash to distinguish the inverted hold. */
inline void scenery(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &, uint32_t elapsed, float a) {
	float angle = (elapsed % 6000) * 0.00104719755f;
	int16_t x = int16_t(cfg.screenW / 2 + cosf(angle) * cfg.screenW / 3);
	int16_t y = int16_t(cfg.screenH / 2 + sinf(angle) * cfg.screenH / 3);
	uint16_t ink = eye_mix(eye_rgb(180, 150, 245), cfg.bgColor, uint8_t(a * 100));
	gfx->fillRoundRect(x, y, cfg.screenW / 30 + 2, cfg.screenH / 100 + 2, 1, ink);
}
/** @brief Register the reversible half-turn without flipping UI or touch. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "upside_down"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(175, 155, 245); p.layout = layout; p.scenery = scenery;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid upside_down face");
} }

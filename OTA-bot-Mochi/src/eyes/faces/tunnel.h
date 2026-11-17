/**
 * @file tunnel.h
 * @brief Relaxed eyes warm briefly as amber tunnel lamps and lane dashes stream past.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace tunnel_face {
/** @brief Left edge of one of three evenly spaced items travelling right to left. */
inline int16_t stream(const EyeConfig &cfg, uint32_t ms, uint8_t index, int16_t width, uint8_t divisor) {
	uint32_t span = uint32_t(cfg.screenW + width);
	return int16_t(cfg.screenW - int32_t((ms / divisor + index * span / 3) % span));
}
/** @brief The nearest lamp to the panel centre sets a temporary warm tint, restored by the fade. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	int16_t lw = cfg.screenW / 6, lh = cfg.screenH / 40 + 2;
	float glow = 0;
	for (uint8_t i = 0; i < 3; ++i) {
		float d = float(stream(cfg, ms, i, lw, 5) + lw / 2 - cfg.screenW / 2) / (cfg.screenW / 2);
		if (d < 0) { d = -d; }
		if (1 - d > glow) { glow = 1 - d; }
	}
	uint16_t warm = eye_mix(eye_rgb(255, 196, 96), cfg.fgColor, uint8_t(glow * glow * 70));
	for (const auto &b : {pair.left, pair.right}) { face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * .28f, warm); }
	if (a <= 0) { return true; }
	uint16_t amber = eye_mix(eye_rgb(255, 176, 64), cfg.bgColor, uint8_t(a * 100));
	uint16_t paint = eye_mix(eye_rgb(210, 214, 222), cfg.bgColor, uint8_t(a * 55));
	int16_t top = pair.left.y < pair.right.y ? pair.left.y : pair.right.y;
	int16_t y = eye_limit(top - cfg.screenH / 7, 1, cfg.screenH - lh - 1);
	int16_t road = eye_limit(pair.left.y + pair.left.h + cfg.screenH / 8, 0, cfg.screenH - 3);
	for (uint8_t i = 0; i < 3; ++i) {
		gfx->fillRoundRect(stream(cfg, ms, i, lw, 5), y, lw, lh, lh / 2, amber);
		gfx->fillRect(stream(cfg, ms, i, lw / 2, 3), road, lw / 2, 2, paint);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("tunnel", scene);
static_assert(face_preset_valid(preset), "Invalid Tunnel face");
} }

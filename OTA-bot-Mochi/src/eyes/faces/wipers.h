/**
 * @file wipers.h
 * @brief Two wiper arms sweep falling raindrops while the eyes follow the blades.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace wipers_face {
/** @brief Rain is timestamp-driven; each arm pivots below its own eye and never leaves the panel. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float swing = sinf((ms % 6000) * .0041887902f);
	for (auto b : {pair.left, pair.right}) {
		b.x = eye_limit(b.x + int16_t(swing * a * cfg.screenW / 40), 0, cfg.screenW - b.w);
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - a * .12f, cfg.fgColor);
	}
	if (a <= 0) { return true; }
	uint16_t rain = eye_mix(eye_rgb(96, 178, 255), cfg.bgColor, uint8_t(a * 100));
	uint16_t blade = eye_mix(eye_rgb(170, 182, 196), cfg.bgColor, uint8_t(a * 100));
	int16_t len = cfg.screenH / 28 + 1;
	for (uint8_t i = 0; i < 10; ++i) {
		int16_t x = int16_t(((i * 37 + 11) % 100) * cfg.screenW / 100);
		int16_t y = int16_t(((ms / 9 + i * 53) % 100) * cfg.screenH / 100);
		gfx->drawLine(x, y, x - len / 3, y + len, rain);
	}
	for (const auto &b : {pair.left, pair.right}) {
		int16_t px = b.x + b.w / 2, py = eye_limit(b.y + b.h + cfg.screenH / 8, 0, cfg.screenH - 1);
		int16_t reach = b.h + cfg.screenH / 6;
		int16_t tx = px + int16_t(sinf(swing * .9f) * reach), ty = py - int16_t(cosf(swing * .9f) * reach);
		for (int16_t t = -1; t <= 1; ++t) { gfx->drawLine(px + t, py, tx + t, ty, blade); }
		gfx->fillCircle(px, py, cfg.screenW / 60 + 1, blade);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("wipers", scene);
static_assert(face_preset_valid(preset), "Invalid Wipers face");
} }

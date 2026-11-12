/**
 * @file origami.h
 * @brief Eye silhouettes fold into creased paper kites, then unfold.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace origami_face {
/** @brief A central crease separates three shaded facets rather than painting an eye accessory. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	float fold = a * (.5f - .5f * cosf((ms % 6000) * .00104719755f));
	for (const auto &b : {pair.left, pair.right}) {
		face_cinematics::body(gfx, cfg, b, a, 1, 1 - fold, cfg.fgColor);
		if (a <= 0) { continue; }
		int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
		int16_t w = int16_t(b.w * .65f * fold), h = int16_t(b.h * .65f * fold);
		uint16_t light = eye_mix(eye_rgb(248, 228, 178), cfg.bgColor, uint8_t(fold * 100));
		uint16_t shade = eye_mix(eye_rgb(112, 172, 214), cfg.bgColor, uint8_t(fold * 100));
		uint16_t edge = eye_mix(eye_rgb(60, 102, 148), cfg.bgColor, uint8_t(fold * 100));
		gfx->fillTriangle(cx, cy - h, cx - w, cy, cx, cy + h, light);
		gfx->fillTriangle(cx, cy - h, cx + w, cy, cx, cy + h, shade);
		gfx->fillTriangle(cx - w, cy, cx, cy + h / 4, cx, cy + h, shade);
		gfx->drawLine(cx, cy - h, cx, cy + h, edge);
		gfx->drawLine(cx - w, cy, cx, cy + h / 4, edge);
	}
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("origami", scene);
static_assert(face_preset_valid(preset), "Invalid Origami face");
} }

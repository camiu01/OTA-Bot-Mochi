/**
 * @file face_cinematics.h
 * @brief Small shared primitives for bounded six-second eye performances.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_support.h"
#include "face_backgrounds.h"
namespace face_cinematics {
/** @brief Preserve the selected style at rest while scaling a scene eye around its center. */
inline void body(Arduino_GFX *gfx, EyeConfig cfg, EyeBox box, float a,
	float sx = 1, float sy = 1, uint16_t tint = 0, float opacity = 1) {
	cfg.fgColor = eye_mix(tint, cfg.fgColor, uint8_t(a * 100));
	cfg.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(opacity * 100));
	int16_t w = int16_t(box.w * sx), h = int16_t(box.h * sy);
	box.x += (box.w - w) / 2; box.y += (box.h - h) / 2;
	box.w = w; box.h = h;
	if (w < 1 || h < 1) { return; }
	box.radius = eye_limit(box.radius, 0, (w < h ? w : h) / 2);
	eye_scene_body(gfx, cfg, box, a);
}
/** @brief Register a complete performance without changing persistent geometry or adding a backdrop. */
constexpr FacePreset preset(const char *name, FaceScene scene) {
	FacePreset p; p.name = name; p.performance = p.fullScene = true;
	p.performanceScene = scene; p.bounds = face_backgrounds::full_bounds;
	return p;
}
}

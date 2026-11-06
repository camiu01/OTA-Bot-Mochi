/**
 * @file devil.h
 * @brief Mischievous style-preserving eyes beneath two external red horns.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_accessories.h"
namespace face_defs { namespace devil_face {
/** @brief Keep a sly upper-lid angle without changing the selected body palette. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	face_primitives::slope(gfx, cfg, b, true, b.h / 5);
}
/** @brief Draw one outlined horn above each eye, with no glyph inside the eye. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &, const EyeLayout &layout) {
	for (const EyeBox &b : {layout.left, layout.right}) {
		int16_t s = eye_limit(13 * eye_scale(cfg) / 100, 6, 28);
		int16_t cx = b.x + b.w / 2, base = face_accessories::top(cfg, b) - 4;
		base = eye_limit(base, 2 * s + 2, cfg.screenH - 2);
		int16_t tip = cx + (b.left ? -s / 2 : s / 2), apex = base - 2 * s;
		gfx->fillTriangle(cx - s, base, cx + s, base, tip, apex, cfg.fgColor);
		int16_t centerX = (2 * cx + tip) / 3, centerY = (2 * base + apex) / 3;
		gfx->fillTriangle((cx - s + centerX) / 2, (base + centerY) / 2,
			(cx + s + centerX) / 2, (base + centerY) / 2,
			(tip + centerX) / 2, (apex + centerY) / 2, eye_rgb(255, 88, 96));
	}
}
/** @brief Retain Devil's stable public ID while replacing the internal horn engraving. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "devil";
	p.left.heightScale = p.right.heightScale = 0.85f;
	p.lids = lids; p.background = background; p.bounds = face_accessories::bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid devil face");
} }

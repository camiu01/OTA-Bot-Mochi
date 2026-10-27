/**
 * @file neon.h
 * @brief Breathing neon outlines with a soft, layered colored glare.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
namespace face_defs { namespace neon_face {
/** @brief Reuse the bounded halo geometry to bloom outward without another canvas. */
inline void glare(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint16_t tube, float a) {
	int16_t step = cfg.screenW / 160 + 1;
	for (int16_t layer = 4; layer > 0; --layer) {
		EyeBox halo = eye_halo_box(cfg, b, layer * step);
		halo.radius = eye_limit(halo.radius, 0, (halo.w < halo.h ? halo.w : halo.h) / 2);
		EyeConfig glow = cfg;
		glow.fgColor = eye_mix(tube, cfg.bgColor, uint8_t(a * (55 - layer * 11)));
		eye_body(gfx, glow, halo);
	}
}
/** @brief Hollow each style's own body without adding pupils. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	for (const EyeBox &b : {layout.left, layout.right}) {
		EyeConfig ink = cfg;
		ink.fgColor = eye_performance_ink(cfg, b.left, elapsed);
		glare(gfx, cfg, b, ink.fgColor, a);
		eye_body(gfx, ink, b);
		int16_t edge = cfg.screenW / 70 + 2;
		EyeBox cut = b;
		cut.w = int16_t((b.w - 2 * edge) * a);
		cut.h = int16_t((b.h - 2 * edge) * a);
		// NORMAL's ellipse subtracts one pixel from each half-size.
		if (cut.w < 2 || cut.h < 2) { continue; }
		cut.x += (b.w - cut.w) / 2; cut.y += (b.h - cut.h) / 2;
		cut.radius = eye_limit(b.radius - edge, 0, (cut.w < cut.h ? cut.w : cut.h) / 2);
		ink.fgColor = cfg.bgColor; eye_body(gfx, ink, cut);
	}
	return true;
}
/** @brief Breathe blue and violet slowly rather than flashing. */
inline uint16_t color(const EyeConfig &, bool left, uint32_t elapsed) {
	return eye_mix(eye_rgb(160, 105, 250), eye_rgb(60, 195, 255),
		uint8_t(50 + 40 * sinf((elapsed % 6000) * 0.0020943951f + (left ? 0 : 1))));
}
/** @brief Register the neon performance. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "neon"; p.performance = p.fullScene = true;
	p.performanceColor = color; p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid neon face");
} }

/**
 * @file glow.h
 * @brief glow definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace glow_face {
using namespace face_primitives;
inline uint16_t local_eye_glow_color(uint32_t now, uint16_t base);


/** @brief glow local_eye_glow_color implementation. */
inline uint16_t local_eye_glow_color(uint32_t now, uint16_t base) {
	static constexpr uint16_t colors[] = {eye_rgb(255, 112, 190), eye_rgb(160, 128, 255),
		eye_rgb(100, 210, 255), eye_rgb(100, 255, 185), eye_rgb(255, 215, 112), eye_rgb(255, 148, 128)};
	uint32_t phase = now % 9600;
	if (phase < 1200) { return base; }
	phase -= 1200;
	uint8_t index = uint8_t(phase / 1200);
	uint16_t from = index == 0 ? base : colors[index - 1];
	uint16_t to = index == 6 ? base : colors[index];
	return eye_mix(to, from, uint8_t((phase % 1200) / 12));
}

/** @brief glow bounds procedure. */
inline EyeLayout bounds(const EyeConfig &cfg, EyeLayout layout) {

		int16_t pad = 6 * eye_scale(cfg) / 100;
		return {eye_halo_box(cfg, layout.left, pad), eye_halo_box(cfg, layout.right, pad)};

}

/** @brief glow background procedure. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {

	cfg.fgColor = local_eye_glow_color(st.lastMs, cfg.fgColor);
	for (int16_t pad = 6; pad >= 2; pad -= 2) {
		EyeConfig halo = cfg;
		halo.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(48 - pad * 6));
		eye_body(gfx, halo, eye_halo_box(cfg, layout.left, pad * eye_scale(cfg) / 100));
		eye_body(gfx, halo, eye_halo_box(cfg, layout.right, pad * eye_scale(cfg) / 100));
	}

}

/** @brief Immutable glow face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "glow";
	p.bounds = bounds;
	p.background = background;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid glow face parameters");
} }

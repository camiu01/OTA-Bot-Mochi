/**
 * @file mecha_mask.h
 * @brief Only the lower mask moves; configured eyes keep their normal live behavior.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "mecha.h"
namespace face_defs { namespace mecha_mask_face {
/** @brief Raise a vented mask below the live eye pair, never painting over their bounds. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	float closed = mecha_face::closure(st.lastMs, MECHA_MASK);
	if (closed <= 0) { return; }
	EyeLayout rest = eye_layout(cfg, EyeState{});
	int16_t bottom = rest.left.y + rest.left.h;
	for (const EyeBox &b : {rest.right, layout.left, layout.right}) {
		if (b.y + b.h > bottom) { bottom = b.y + b.h; }
	}
	int16_t top = eye_limit(int16_t(bottom * 100 / (cfg.screenH - 1) + 3), 50, 94);
	int16_t depth = int16_t(100 - top);
	int16_t drop = int16_t((depth + 3) * (1 - closed));
	auto ink = [&](uint16_t color) { return eye_mix(color, cfg.bgColor, uint8_t(st.backdropOpacity * 100)); };
	for (bool right : {false, true}) {
		mecha_face::quad(gfx, cfg, right, 0, 9, top, 28, top + depth / 4,
			50, top, 50, 98, ink(eye_rgb(187, 209, 222)), drop);
		mecha_face::quad(gfx, cfg, right, 0, 9, top, 15, top + 2,
			19, 96, 12, 92, ink(eye_rgb(112, 145, 165)), drop);
		mecha_face::quad(gfx, cfg, right, 0, 14, top + 2, 31, top + depth / 3,
			30, top + depth / 3 + 2, 14, top + 4, ink(eye_rgb(224, 127, 68)), drop);
		for (int16_t i = 1; i <= 3; ++i) {
			int16_t y = int16_t(top + depth * i / 5);
			mecha_face::quad(gfx, cfg, right, 0, 39, y + 3, 49, y,
				49, y + 1, 39, y + 4, ink(eye_rgb(25, 35, 46)), drop);
		}
	}
}

/** @brief Keep the standard eye renderer and animate only the lower mask. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "mecha_mask";
	p.background = background; p.bounds = face_backgrounds::full_bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid lower-mask mecha face");
} }

/**
 * @file angel.h
 * @brief Gentle eyes with an external golden aureole and paired feathered wings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_accessories.h"
namespace face_defs { namespace angel_face {
/** @brief A small lower-lid smile keeps every configured eye style recognizable. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	face_primitives::smile(gfx, cfg, b, b.h / 10, b.h / 8);
}
/** @brief Three rounded feathers on each outer side, never a mouth or body. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	face_accessories::aureole(gfx, cfg, st, layout);
	uint16_t featherInk = face_accessories::ink(cfg, st, eye_rgb(187, 221, 247));
	int16_t scale = eye_scale(cfg);
	for (const EyeBox &b : {layout.left, layout.right}) {
		int16_t side = b.left ? -1 : 1;
		int16_t edge = b.left ? b.x - 2 : b.x + b.w + 2;
		int16_t room = b.left ? edge : cfg.screenW - edge;
		if (room < 6) { continue; }
		int16_t feather = eye_limit(4 * scale / 100, 1, (room - 2) / 3);
		int16_t length = eye_limit(24 * scale / 100, 2, room - feather - 2);
		int16_t cy = b.y + b.h / 2;
		for (int16_t i = 0; i < 3; ++i) {
			int16_t reach = length - i * length / 5;
			int16_t x = edge + side * reach;
			int16_t y = cy - (2 - i) * feather * 2;
			gfx->fillTriangle(edge, cy + feather, x, y - feather, x, y + feather,
				featherInk);
			gfx->fillCircle(x, y, feather, featherInk);
		}
	}
}
/** @brief Appended Angel definition; Halo remains a separate aureole-only face. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "angel"; p.lids = lids;
	p.background = background; p.bounds = face_accessories::bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid angel face");
} }

/**
 * @file arc_sad.h
 * @brief arc_sad definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace arc_sad_face {
using namespace face_primitives;

/** @brief arc_sad lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {

	for (int16_t i = 0; i < b.w; ++i) {
		int16_t top = 0, bottom = 0;
		if (cfg.expression == TILTED) {
			int16_t along = b.left ? i : b.w - 1 - i;
			top = along * b.h / (b.w * 3);
			bottom = (b.w - 1 - along) * b.h / (b.w * 3);
		} else {
			int32_t t = 2 * i - b.w + 1;
			int16_t curve = int16_t(t * t * b.h / (int32_t(b.w) * b.w * 3));
			top = cfg.expression == ARC_HAPPY ? b.h / 5 + curve : b.h * 3 / 5 - curve;
			bottom = b.h - top - (b.h / 9 + 2);
		}
		if (top > 0) { gfx->fillRect(b.x + i, b.y, 1, top, cfg.bgColor); }
		if (bottom > 0) { gfx->fillRect(b.x + i, b.y + b.h - bottom, 1, bottom, cfg.bgColor); }
	}

}

/** @brief Immutable arc_sad face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "arc_sad";
	p.left.heightScale = 0.55f;
	p.right.heightScale = 0.55f;
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid arc_sad face parameters");
} }

/**
 * @file squeezed.h
 * @brief squeezed definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace squeezed_face {
using namespace face_primitives;

/** @brief squeezed referenceBody procedure. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)st; (void)elapsed; (void)neutral;

	int16_t edge = b.left ? b.x + 3 : b.x + b.w - 4;
	int16_t tip = b.left ? b.x + b.w - 4 : b.x + 3;
	int16_t thickness = b.h / 14 + 1;
	for (int16_t i = 0; i < thickness; ++i) {
		gfx->drawLine(edge, b.y + b.h / 5 + i, tip, b.y + b.h / 2 + i, cfg.fgColor);
		gfx->drawLine(tip, b.y + b.h / 2 + i, edge, b.y + b.h * 4 / 5 + i, cfg.fgColor);
	}
	return true;

}

/** @brief Immutable squeezed face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "squeezed";
	p.left.heightScale = 0.60f;
	p.right.heightScale = 0.60f;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid squeezed face parameters");
} }

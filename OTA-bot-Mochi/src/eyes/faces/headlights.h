/**
 * @file headlights.h
 * @brief headlights definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace headlights_face {
using namespace face_primitives;
inline void local_eye_headlight(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t now);


/** @brief headlights local_eye_headlight implementation. */
inline void local_eye_headlight(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t now) {
	int16_t reach = b.left ? b.x : cfg.screenW - b.x - b.w;
	if (reach <= 0) { return; }
	float opening = now >= 1200 ? 1 : now / 1200.0f;
	opening = opening * opening * (3 - 2 * opening);
	int16_t tip = int16_t(reach * opening);
	if (tip == 0) { return; }
	float wave = sinf((now % 2400) * 0.0026179939f + (b.left ? 0.0f : 1.5707963f));
	int16_t spread = b.h / 4 + 16 * eye_scale(cfg) / 100;
	int16_t cy = b.y + b.h / 2;
	const uint16_t soft = eye_mix(eye_rgb(255, 255, 255), cfg.fgColor, 30);
	// Extend beneath the eye so rounded corners meet the beam without a black seam.
	for (int16_t d = -b.w / 2; d <= tip; ++d) {
		int16_t half = b.h / 2 + int32_t(d) * spread / reach;
		if (half < 0) { continue; }
		int16_t x = b.left ? b.x - d : b.x + b.w - 1 + d;
		int32_t fade = d < 0 ? 100 : 100 - int32_t(d) * 100 / reach;
		fade = 25 + fade * 3 / 4;
		fade = int32_t(fade * opening);
		if (opening < 1 && d > tip - 6 && d > 0) { fade = fade * (tip - d) / 6; }
		for (int16_t layer = 0; layer < 8; ++layer) {
			int16_t inner = half - half * layer / 32;
			uint8_t amount = uint8_t((20 + layer * 8 + 6 * wave) * fade / 100);
			int16_t top = eye_limit(cy - inner, 0, cfg.screenH - 1);
			int16_t bottom = eye_limit(cy + inner + 1, top + 1, cfg.screenH);
			gfx->fillRect(x, top, 1, bottom - top, eye_mix(soft, cfg.bgColor, amount));
		}
	}
}

/** @brief headlights bounds procedure. */
inline EyeLayout bounds(const EyeConfig &cfg, EyeLayout layout) {

		EyeBox *boxes[] = {&layout.left, &layout.right};
		for (EyeBox *b : boxes) {
			int16_t half = b->h * 3 / 4 + 20 * eye_scale(cfg) / 100 + 3;
			int16_t cy = b->y + b->h / 2;
			int16_t top = eye_limit(cy - half, 0, cfg.screenH - 1);
			int16_t bottom = eye_limit(cy + half + 1, top + 1, cfg.screenH);
			b->x = 0; b->w = cfg.screenW; b->y = top; b->h = bottom - top;
		}

	return layout;
}

/** @brief headlights background procedure. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	local_eye_headlight(gfx, cfg, layout.left, st.lastMs);
	local_eye_headlight(gfx, cfg, layout.right, st.lastMs);
}

/** @brief Immutable headlights face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "headlights";
	p.bounds = bounds;
	p.background = background;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid headlights face parameters");
} }

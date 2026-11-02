/**
 * @file loading.h
 * @brief Decorative loading eyes with a rotating cyan dot trail.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace loading_face {
/** @brief Draw a bounded spinner on a dark inset without obscuring it with pupils. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral;
	eye_body(gfx, cfg, b);
	int16_t half = int16_t((b.w < b.h ? b.w : b.h) / 2);
	int16_t dot = eye_limit(int16_t(half / 7), 1, 6);
	int16_t radius = int16_t(half - dot - 3);
	if (radius < 3) { return true; }
	int16_t x = b.x + b.w / 2, y = b.y + b.h / 2;
	gfx->fillCircle(x, y, radius + dot + 1, cfg.bgColor);
	uint8_t head = uint8_t(st.lastMs / 180 % 8);
	for (uint8_t i = 0; i < 8; ++i) {
		uint8_t age = uint8_t((head + 8 - i) % 8);
		uint8_t amount = age == 0 ? 100 : uint8_t(20 + (7 - age) * 8);
		float angle = i * 0.78539816f;
		gfx->fillCircle(int16_t(x + sinf(angle) * radius), int16_t(y - cosf(angle) * radius),
			dot, eye_mix(eye_rgb(64, 212, 255), cfg.bgColor, amount));
	}
	return true;
}

/** @brief Register a continuous decorative spinner without changing persisted settings. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "loading";
	p.normalCycle = true;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid loading face parameters");
} }

/**
 * @file fire.h
 * @brief fire definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace fire_face {
using namespace face_primitives;

/** @brief Stable pseudo-random samples, independent of drawing order and global RNG. */
inline float noise(uint32_t tick, uint32_t seed) {
	uint32_t value = tick + seed;
	value ^= value >> 16; value *= 0x7feb352du;
	value ^= value >> 15; value *= 0x846ca68bu;
	value ^= value >> 16;
	return (value & 65535) / 65535.0f;
}

/** @brief Smoothly interpolate irregular flicker rather than jumping between random frames. */
inline float flicker(uint32_t time, uint32_t seed) {
	uint32_t tick = time / 130;
	float t = (time % 130) / 130.0f;
	t = t * t * (3 - 2 * t);
	return noise(tick, seed) * (1 - t) + noise(tick + 1, seed) * t;
}

/** @brief Animate three independently flickering tongues within the advertised eye bounds. */
inline bool referenceBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st,
		const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)elapsed; (void)neutral; (void)cfg;
	int16_t s = eye_limit((b.w < b.h ? b.w : b.h) / 2 - 2, 3, 60);
	int16_t x = b.x + b.w / 2, y = b.y + b.h / 2;
	uint32_t seed = b.left ? 173u : 947u;
	for (uint8_t tongue = 0; tongue < 3; ++tongue) {
		float height = flicker(st.lastMs, seed + tongue * 101u);
		int16_t radius = int16_t(s * (0.48f + height * 0.28f));
		int16_t dx = int16_t((flicker(st.lastMs, seed + tongue * 227u + 19u) - 0.5f) * s * 0.4f);
		int16_t dy = int16_t(s - radius);
		flame(gfx, x + dx, y + dy, radius, eye_rgb(255, 112, 32));
		if (radius > 7) { flame(gfx, x + dx, y + dy + radius / 3, radius / 2, eye_rgb(255, 224, 64)); }
	}
	return true;
}

/** @brief Keep flicker tied to the selected face's clock. */
inline void drawing(EyeState &st, const EyeLayout &, uint32_t elapsed) {
	st.lastMs = elapsed;
}

/** @brief Immutable fire face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "fire";
	p.keepOpen = true;
	p.drawing = drawing;
	p.left.widthDelta = p.right.widthDelta = 12;
	p.left.heightFromWidth = p.right.heightFromWidth = true;
	p.wholeEye = p.replacesSolid = true;
	p.referenceBody = referenceBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid fire face parameters");
} }

/**
 * @file crying.h
 * @brief crying definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace crying_face {
using namespace face_primitives;
inline float local_eye_crying_ease(float t);
inline float local_eye_crying_closure(uint32_t elapsed);
inline void local_eye_crying_lid(Arduino_GFX *gfx, EyeConfig cfg, const EyeState &st, const EyeBox &b, float closure);
inline void local_eye_crying_water_rows(const EyeBox &b, uint32_t elapsed, int16_t &top, int16_t &begin, int16_t &end);
inline void local_eye_crying_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, uint16_t neutral);
inline void local_eye_reaction_tears(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral);


/** @brief crying lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, false, b.h / 3);
}

/** @brief crying glyph procedure. */
inline bool glyph(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	(void)cfg; (void)b; (void)x; (void)y; (void)s; (void)cut;
	tear(gfx, x, tear_y(b), 2, color); return true;
}

/** @brief crying reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = CRYING; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	sx += 0.28f * local_eye_crying_closure(phase);
			dy = a * 3 * sinf(phase * 0.0041887902f);
}

/** @brief crying local_eye_crying_ease implementation. */
inline float local_eye_crying_ease(float t) {
	t = t < 0 ? 0 : (t > 1 ? 1 : t);
	return t * t * (3 - 2 * t);
}

/** @brief crying local_eye_crying_closure implementation. */
inline float local_eye_crying_closure(uint32_t elapsed) {
	uint32_t p = elapsed % 3000;
	return local_eye_crying_ease(p < 250 ? 0 : (p < 650 ? (p - 250) / 400.0f :
		(p < 2500 ? 1 : (p < 2850 ? (2850 - p) / 350.0f : 0))));
}

/** @brief crying local_eye_crying_lid implementation. */
inline void local_eye_crying_lid(Arduino_GFX *gfx, EyeConfig cfg, const EyeState &st, const EyeBox &b, float closure) {
	int16_t lid = eye_limit(b.h / 9, 3, 9);
	EyeBox closing = b;
	closing.h = int16_t(b.h + (lid - b.h) * closure);
	int16_t center = b.y + b.h / 2;
	int16_t target = b.y + b.h / 6 + lid / 2;
	closing.y = int16_t(center + (target - center) * closure) - closing.h / 2;
	closing.radius = eye_limit(b.radius, 0, (closing.w < closing.h ? closing.w : closing.h) / 2);
	cfg.expression = NEUTRAL;
	if (closure == 1 || closing.h < 4) {
		gfx->fillRoundRect(closing.x, closing.y, closing.w, closing.h, closing.h / 2, cfg.fgColor);
	} else {
		eye_body(gfx, cfg, closing);
		if (closing.h > 13) { eye_pupils(gfx, cfg, st, closing); }
	}
}

/** @brief crying local_eye_crying_water_rows implementation. */
inline void local_eye_crying_water_rows(const EyeBox &b, uint32_t elapsed, int16_t &top, int16_t &begin, int16_t &end) {
	uint32_t phase = elapsed % 3000;
	int16_t lid = eye_limit(b.h / 9, 3, 9);
	top = b.y + b.h / 6 + lid / 2;
	int16_t available = eye_limit(b.y + b.h - 2 - top, 0, b.h);
	float growth = local_eye_crying_ease(phase < 500 ? 0 : (phase - 500) / 750.0f);
	float drain = local_eye_crying_ease(phase < 2200 ? 0 : (phase - 2200) / 300.0f);
	begin = int16_t(available * drain);
	end = int16_t(available * growth);
}

/** @brief crying local_eye_crying_water implementation. */
inline void local_eye_crying_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	int16_t top, begin, end;
	local_eye_crying_water_rows(b, elapsed, top, begin, end);
	float phase = float(elapsed % 3000);
	for (int16_t row = begin; row < end; ++row) {
		float wave = sinf(row * 0.18f - phase * 0.0034906585f);
		int16_t width = int16_t(b.w * (0.84f + 0.035f * wave));
		int16_t x = b.x + (b.w - width) / 2 + int16_t(wave * b.w / 20);
		float stripe = 0.5f + 0.5f * sinf(row * 0.32f - phase * 0.008f);
		uint16_t ink = eye_mix(neutral, cfg.fgColor, uint8_t(6 + 20 * stripe * stripe * stripe));
		gfx->fillRect(x, top + row, width, 1, ink);
	}
}

/** @brief crying local_eye_reaction_tears implementation. */
inline void local_eye_reaction_tears(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	local_eye_crying_water(gfx, cfg, b, elapsed, neutral);
	EyeConfig lids = cfg;
	lids.fgColor = neutral;
	local_eye_crying_lid(gfx, lids, st, b, local_eye_crying_closure(elapsed));
}

/** @brief crying reactionBody procedure. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	local_eye_reaction_tears(gfx, cfg, st, b, elapsed, neutral);
	return true;
}

/** @brief Immutable crying face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "crying";
	p.left.heightScale = 0.85f;
	p.right.heightScale = 0.85f;
	p.hasReactionInk = true;
	p.reactionInk = eye_rgb(75, 205, 255);
	p.lids = lids;
	p.glyph = glyph;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid crying face parameters");
} }

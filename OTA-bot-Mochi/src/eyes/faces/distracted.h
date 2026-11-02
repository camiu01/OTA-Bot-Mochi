/**
 * @file distracted.h
 * @brief distracted definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_random.h"

namespace face_defs { namespace distracted_face {
using namespace face_primitives;

/** @brief distracted performanceMotion procedure. */
inline void performanceMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)sx; (void)sy;
	uint32_t time = phase + (left ? 0u : 270u), event = time / 1100 + 1;
	float t = (time % 1100) / 240.0f;
	if (t > 1) { t = 1; }
	t = t * t * (3 - 2 * t);
	uint32_t salt = left ? 173u : 947u;
	auto target = [&](uint32_t e, uint32_t s) { return face_random(e, s) * 2 - 1; };
	dx = a * 14 * (target(event - 1, salt) * (1 - t) + target(event, salt) * t);
	dy = a * 6 * (target(event - 1, salt + 37) * (1 - t) + target(event, salt + 37) * t);
}

/** @brief Preserve the full entry-relative event clock across repeated six-second performances. */
inline void layout(const EyeConfig &cfg, EyeBox &b, uint32_t elapsed, float a) {
	float sx = 1, sy = 1, dx = 0, dy = 0;
	performanceMotion(b.left, elapsed, a, sx, sy, dx, dy);
	b.x = eye_limit(int16_t(b.x + dx * eye_scale(cfg) / 100), 0, cfg.screenW - b.w);
	b.y = eye_limit(int16_t(b.y + dy * eye_scale(cfg) / 100), 0, cfg.screenH - b.h);
}
/** @brief Immutable distracted face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "distracted";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(130, 225, 210);
	p.performanceMotion = performanceMotion;
	p.layout = layout;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid distracted face parameters");
} }

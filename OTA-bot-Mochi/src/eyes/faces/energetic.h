/**
 * @file energetic.h
 * @brief energetic definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace energetic_face {
using namespace face_primitives;
inline void local_eye_turbo_impeller(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx,
		int16_t cy, int16_t rx, int16_t ry, float angle, float a);
inline EyeBox local_eye_turbo_target(const EyeConfig &cfg, bool left);
inline void local_eye_turbo_smoke(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy,
		int16_t r, bool left, uint32_t elapsed, float a);
inline void local_eye_performance_turbo(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a);


/** @brief energetic local_eye_turbo_impeller implementation. */
inline void local_eye_turbo_impeller(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx,
		int16_t cy, int16_t rx, int16_t ry, float angle, float a) {
	uint16_t face = eye_mix(eye_rgb(175, 205, 225), cfg.bgColor, uint8_t(a * 100));
	uint16_t shade = eye_mix(eye_rgb(80, 130, 165), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 8; ++i) {
		float t = angle + i * 0.785398163f;
		int16_t ix = int16_t(cx + cosf(t) * rx * 0.2f), iy = int16_t(cy + sinf(t) * ry * 0.2f);
		int16_t ox = int16_t(cx + cosf(t + 0.42f) * rx), oy = int16_t(cy + sinf(t + 0.42f) * ry);
		int16_t tx = int16_t(cx + cosf(t + 0.75f) * rx * 0.9f), ty = int16_t(cy + sinf(t + 0.75f) * ry * 0.9f);
		gfx->fillTriangle(ix, iy, ox, oy, tx, ty, face);
		gfx->fillTriangle(ix, iy, tx, ty, int16_t(cx + cosf(t + 0.5f) * rx * 0.42f),
			int16_t(cy + sinf(t + 0.5f) * ry * 0.42f), shade);
	}
	gfx->fillEllipse(cx, cy, rx / 5 + 1, ry / 5 + 1,
		eye_mix(eye_rgb(225, 175, 105), cfg.bgColor, uint8_t(a * 100)));
}

/** @brief energetic local_eye_turbo_target implementation. */
inline EyeBox local_eye_turbo_target(const EyeConfig &cfg, bool left) {
	int16_t r = cfg.screenW / 6;
	if (r > cfg.screenH / 4) { r = cfg.screenH / 4; }
	int16_t cx = cfg.screenW * (left ? 1 : 3) / 4;
	return {int16_t(cx - r), int16_t(cfg.screenH / 2 - r), int16_t(r * 2), int16_t(r * 2), r, left};
}

/** @brief energetic local_eye_turbo_smoke implementation. */
inline void local_eye_turbo_smoke(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy,
		int16_t r, bool left, uint32_t elapsed, float a) {
	int16_t direction = left ? -1 : 1, pipeH = int16_t(r * 0.32f * a), pipeW = int16_t(r * 0.6f * a);
	int16_t end = cx + direction * int16_t(r * 1.35f * a);
	if (pipeH > 0 && pipeW > 0) {
		gfx->fillRect(direction < 0 ? end : end - pipeW, cy + r / 4, pipeW, pipeH,
			eye_mix(eye_rgb(100, 115, 130), cfg.bgColor, uint8_t(a * 100)));
	}
	uint32_t phase = elapsed % 1500;
	for (uint8_t i = 0; i < 3; ++i) {
		if (phase < i * 100u || phase >= i * 100u + 900u) { continue; }
		float t = (phase - i * 100u) / 900.0f;
		int16_t size = int16_t(r * (0.1f + 0.24f * t) * a);
		if (size < 1) { continue; }
		int16_t x = eye_limit(int16_t(end + direction * t * r * 0.3f), size, cfg.screenW - size - 1);
		int16_t y = int16_t(cy + r / 4 - t * r * 1.6f);
		uint16_t ink = eye_mix(eye_rgb(175, 185, 195), cfg.bgColor, uint8_t(a * (1 - t) * 70));
		gfx->fillCircle(x, y, size, ink);
		gfx->fillCircle(x - direction * size / 2, y - size / 2, size * 3 / 4, ink);
	}
}

/** @brief energetic local_eye_performance_turbo implementation. */
inline void local_eye_performance_turbo(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	EyeBox target = local_eye_turbo_target(cfg, b.left), housing = eye_scene_box(b, target, a);
	int16_t cx = housing.x + housing.w / 2, cy = housing.y + housing.h / 2, r = target.radius;
	int16_t direction = b.left ? 1 : -1;
	local_eye_turbo_smoke(gfx, cfg, cx, cy, r, b.left, elapsed, a);
	uint16_t steel = eye_rgb(145, 165, 185);
	uint16_t casing = eye_mix(steel, cfg.bgColor, uint8_t(a * 100));
	int16_t tubeW = int16_t(r * 1.3f * a), tubeH = int16_t(r * 0.48f * a);
	if (tubeW > 2 && tubeH > 2) {
		int16_t x = direction > 0 ? cx : cx - tubeW;
		gfx->fillRoundRect(x, cy - r, tubeW, tubeH, tubeH / 3, casing);
		gfx->fillRect(direction > 0 ? cx + tubeW - tubeH / 3 : x, cy - r - tubeH / 6, tubeH / 3 + 1,
			tubeH * 4 / 3, eye_mix(eye_rgb(205, 215, 225), cfg.bgColor, uint8_t(a * 100)));
	}
	cfg.fgColor = eye_mix(steel, cfg.fgColor, uint8_t(a * 100));
	eye_scene_body(gfx, cfg, housing, a);
	int16_t faceX = cx + direction * int16_t(r * 0.18f * a);
	int16_t rx = int16_t(r * 0.54f * a), ry = int16_t(r * 0.72f * a), rim = int16_t(a * 3);
	gfx->fillEllipse(faceX, cy, rx + rim, ry + rim,
		eye_mix(eye_rgb(70, 90, 110), cfg.fgColor, uint8_t(a * 100)));
	gfx->fillEllipse(faceX, cy, rx, ry, cfg.bgColor);
	local_eye_turbo_impeller(gfx, cfg, faceX, cy, rx, ry,
		(elapsed % 6000) * (elapsed % 6000) * 0.000003f * direction, a);
	uint16_t trim = eye_mix(eye_rgb(205, 220, 230), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 4; ++i) {
		float t = 0.785398163f + i * 1.570796327f;
		gfx->fillCircle(int16_t(cx + cosf(t) * r * 0.9f), int16_t(cy + sinf(t) * r * 0.9f),
			int16_t((r / 18 + 1) * a), trim);
	}
}

/** @brief energetic performanceScene procedure. */
inline bool performanceScene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &b : eyes) { local_eye_performance_turbo(gfx, cfg, b, elapsed, a); }
	return true;
}

/** @brief Immutable energetic face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "energetic";
	p.performance = true;
	p.fullScene = true;
	p.performanceInk = eye_rgb(120, 175, 210);
	p.performanceScene = performanceScene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid energetic face parameters");
} }

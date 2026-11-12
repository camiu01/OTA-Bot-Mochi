/**
 * @file mecha_break.h
 * @brief Mecha mask cracks and explodes into spinning armor fragments above the real eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "mecha.h"
namespace face_defs { namespace mecha_break_face {
/** @brief One alloy panel in the same normalized coordinates as the assembled mask. */
struct Shard { int16_t x[4], y[4]; uint16_t ink; };

/** @brief Launch an actual armor panel radially, with a repeatable tumble and downward arc. */
inline void fragment(Arduino_GFX *gfx, const EyeConfig &cfg, const Shard &shard, bool right, float t) {
	float cx = 0, cy = 0;
	for (uint8_t i = 0; i < 4; ++i) { cx += shard.x[i] / 4.0f; cy += shard.y[i] / 4.0f; }
	float travel = sqrtf(t), angle = t * (right ? -1 : 1) * (1.6f + cx / 30);
	float c = cosf(angle), s = sinf(angle);
	float dx = (cx - 50) * travel * 3.2f, dy = (cy - 50) * travel * 3.2f + t * t * 38;
	int16_t x[4], y[4];
	for (uint8_t i = 0; i < 4; ++i) {
		float u = shard.x[i] - cx, v = shard.y[i] - cy;
		float px = cx + dx + u * c - v * s;
		x[i] = int16_t((right ? 100 - px : px) * (cfg.screenW - 1) / 100);
		y[i] = int16_t((cy + dy + u * s + v * c) * (cfg.screenH - 1) / 100);
	}
	uint8_t opacity = t < 0.7f ? 100 : uint8_t((1 - t) * 333);
	uint16_t ink = eye_mix(shard.ink, cfg.bgColor, opacity);
	gfx->fillTriangle(x[0], y[0], x[1], y[1], x[2], y[2], ink);
	gfx->fillTriangle(x[0], y[0], x[2], y[2], x[3], y[3], ink);
}

/** @brief Break the forehead, copper optics, cheeks and chin into independent mirrored panels. */
inline void debris(Arduino_GFX *gfx, const EyeConfig &cfg, float t) {
	uint16_t steel = eye_rgb(187, 209, 222), pale = eye_rgb(210, 226, 234);
	uint16_t shade = eye_rgb(112, 145, 165), copper = eye_rgb(224, 127, 68);
	const Shard shards[] = {
		{{12,40,41,13}, {5,2,30,20}, steel}, {{15,41,43,14}, {22,33,37,27}, shade},
		{{15,43,43,15}, {28,39,43,33}, copper}, {{19,41,33,22}, {38,45,47,43}, eye_rgb(247,174,84)},
		{{44,49,49,44}, {1,1,46,41}, pale}, {{10,15,19,12}, {35,38,73,70}, shade},
		{{12,17,20,14}, {50,52,65,62}, pale}, {{15,41,43,22}, {48,56,73,64}, steel},
		{{17,32,31,18}, {50,57,61,56}, copper}, {{23,42,42,26}, {67,76,82,76}, shade},
		{{44,49,49,44}, {47,48,70,67}, pale}, {{34,49,49,34}, {82,82,96,94}, steel}
	};
	for (bool right : {false, true}) {
		for (const auto &shard : shards) { fragment(gfx, cfg, shard, right, t); }
	}
}

/** @brief A localized white-hot impact, expanding shock ring and short copper sparks, never a screen flash. */
inline void blast(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t age) {
	if (age >= 700) { return; }
	float t = age / 700.0f;
	int16_t cx = cfg.screenW / 2, cy = cfg.screenH * 47 / 100;
	int16_t span = cfg.screenW < cfg.screenH ? cfg.screenW : cfg.screenH;
	uint16_t ink = eye_mix(eye_rgb(255, 182, 76), cfg.bgColor, uint8_t((1 - t) * 100));
	int16_t radius = int16_t(span * (0.035f + t * 0.48f));
	gfx->drawCircle(cx, cy, radius, ink);
	for (uint8_t i = 0; i < 12; ++i) {
		float angle = i * 0.523598776f;
		float length = radius * (i & 1 ? 1.2f : 0.85f);
		gfx->drawLine(cx + int16_t(cosf(angle) * length), cy + int16_t(sinf(angle) * length),
			cx + int16_t(cosf(angle) * (length + span / 18)), cy + int16_t(sinf(angle) * (length + span / 18)), ink);
	}
	if (age < 140) {
		gfx->fillCircle(cx, cy, int16_t(span * 0.055f * (1 - age / 140.0f)) + 1,
			eye_mix(eye_rgb(255, 246, 210), cfg.bgColor, uint8_t((140 - age) * 100 / 140)));
	}
}

/** @brief Assemble the familiar mask, crack its seams, then explode it to uncover the configured style. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	uint32_t phase = elapsed % 6000;
	if (phase < 2600) { return mecha_face::scene(gfx, cfg, layout, elapsed, a); }
	EyeConfig original = cfg; original.expression = NEUTRAL;
	for (const EyeBox &b : {layout.left, layout.right}) {
		eye_body(gfx, original, b); eye_pupils(gfx, original, EyeState{}, b);
	}
	uint32_t age = phase - 2600;
	if (age < 80) {
		mecha_face::helmet(gfx, cfg, 1, false);
		uint16_t crack = eye_rgb(255, 213, 124);
		for (bool right : {false, true}) {
			int16_t x = cfg.screenW / 2, y = cfg.screenH * 47 / 100, sign = right ? 1 : -1;
			gfx->drawLine(x, y, x + sign * cfg.screenW / 10, y - cfg.screenH / 9, crack);
			gfx->drawLine(x + sign * cfg.screenW / 10, y - cfg.screenH / 9,
				x + sign * cfg.screenW / 6, y - cfg.screenH / 4, crack);
		}
	} else if (age < 1400) { debris(gfx, cfg, (age - 80) / 1320.0f); }
	blast(gfx, cfg, age);
	return true;
}
/** @brief Reuse the mask's identity and full dirty bounds, not its orderly opening animation. */
inline constexpr FacePreset preset = [] {
	FacePreset p = mecha_face::preset;
	p.name = "mecha_break"; p.scene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid breakaway mecha face");
} }

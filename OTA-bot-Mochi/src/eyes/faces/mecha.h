/**
 * @file mecha.h
 * @brief Segmented pale alloy armor with copper optics and dark mechanical joints.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_backgrounds.h"
namespace face_defs { namespace mecha_face {
/** @brief Draw a mirrored normalized quad, keeping its geometry panel-relative. */
inline void quad(Arduino_GFX *gfx, const EyeConfig &cfg, bool right, int16_t shift,
		int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		int16_t x3, int16_t y3, int16_t x4, int16_t y4, uint16_t ink, int16_t lift = 0) {
	auto x = [&](int16_t n) { return int16_t((right ? 100 - n - shift : n + shift) * (cfg.screenW - 1) / 100); };
	auto y = [&](int16_t n) { return int16_t((n + lift) * (cfg.screenH - 1) / 100); };
	gfx->fillTriangle(x(x1), y(y1), x(x2), y(y2), x(x3), y(y3), ink);
	gfx->fillTriangle(x(x1), y(y1), x(x3), y(y3), x(x4), y(y4), ink);
}

/** @brief Close over the original eyes, hold the complete helmet, then uncover them. */
inline float closure(uint32_t elapsed, Expression variant) {
	uint32_t phase = elapsed % 6000;
	bool earlyOpen = variant == MECHA_MASK;
	uint32_t shut = earlyOpen ? 1400 : 1800, reopen = earlyOpen ? 2200 : 3600;
	uint32_t end = earlyOpen ? 3400 : 4800;
	if (variant == MECHA_BREAK) { reopen = 2600; end = 4600; }
	float t = phase <= 900 || phase >= end ? 0 :
		phase < shut ? (phase - 900) / float(shut - 900) :
		phase <= reopen ? 1 : (end - phase) / float(end - reopen);
	return t * t * (3 - 2 * t);
}

/** @brief Split forehead plates and copper optics retract as a separate assembly. */
inline void visor(Arduino_GFX *gfx, const EyeConfig &cfg, float closed) {
	int16_t lift = int16_t(-48 * (1 - closed));
	int16_t shift = cfg.expression == MECHA_BREAK ? int16_t(-30 * (1 - closed)) : 0;
	for (bool right : {false, true}) {
		uint16_t dark = eye_rgb(25, 35, 46), steel = eye_rgb(187, 209, 222);
		quad(gfx, cfg, right, shift, 8, 3, 50, 0, 50, 48, 7, 46, dark, lift);
		quad(gfx, cfg, right, shift, 12, 5, 40, 2, 41, 30, 13, 20, steel, lift);
		quad(gfx, cfg, right, shift, 15, 22, 41, 33, 43, 37, 14, 27, eye_rgb(112, 145, 165), lift);
		quad(gfx, cfg, right, shift, 15, 28, 43, 39, 43, 43, 15, 33, eye_rgb(224, 127, 68), lift);
		quad(gfx, cfg, right, shift, 19, 38, 41, 45, 33, 47, 22, 43, eye_rgb(247, 174, 84), lift);
		quad(gfx, cfg, right, shift, 44, 1, 49, 1, 49, 46, 44, 41, eye_rgb(210, 226, 234), lift);
		quad(gfx, cfg, right, shift, 41, 4, 42, 4, 42, 28, 41, 27, dark, lift);
		for (int16_t row = 9; row <= 23; row += 7) {
			quad(gfx, cfg, right, shift, 46, row, 49, row, 49, row + 1, 46, row + 1, dark, lift);
		}
	}
}

/** @brief Left and right cheek guards slide laterally, independently of the visor. */
inline void cheeks(Arduino_GFX *gfx, const EyeConfig &cfg, float closed) {
	int16_t shift = int16_t(-16 * (1 - closed));
	for (bool right : {false, true}) {
		quad(gfx, cfg, right, shift, 4, 28, 16, 32, 20, 84, 9, 76, eye_rgb(25, 35, 46));
		quad(gfx, cfg, right, shift, 10, 35, 15, 38, 19, 73, 12, 70, eye_rgb(112, 145, 165));
		quad(gfx, cfg, right, shift, 12, 50, 17, 52, 20, 65, 14, 62, eye_rgb(210, 226, 234));
		for (int16_t col = 4; col <= 8; col += 2) {
			quad(gfx, cfg, right, shift, col, 34, col + 1, 32, col + 1, 55, col, 57, eye_rgb(151, 181, 199));
		}
	}
}

/** @brief The lower mask and chin descend together, uncovering the original eye row. */
inline void jaw(Arduino_GFX *gfx, const EyeConfig &cfg, float closed) {
	int16_t drop = int16_t(56 * (1 - closed));
	int16_t shift = cfg.expression == MECHA_BREAK ? int16_t(-30 * (1 - closed)) : 0;
	uint16_t shell = eye_rgb(25, 35, 46), shade = eye_rgb(112, 145, 165);
	uint16_t blue = eye_rgb(187, 209, 222);
	for (bool right : {false, true}) {
		quad(gfx, cfg, right, shift, 12, 47, 50, 47, 50, 98, 25, 90, shell, drop);
		quad(gfx, cfg, right, shift, 15, 48, 41, 56, 43, 73, 22, 64, blue, drop);
		quad(gfx, cfg, right, shift, 17, 50, 32, 57, 31, 61, 18, 56, eye_rgb(224, 127, 68), drop);
		quad(gfx, cfg, right, shift, 23, 67, 42, 76, 42, 82, 26, 76, shade, drop);
		quad(gfx, cfg, right, shift, 44, 47, 49, 48, 49, 70, 44, 67, eye_rgb(210, 226, 234), drop);
		quad(gfx, cfg, right, shift, 34, 82, 49, 82, 49, 96, 34, 94, blue, drop);
		quad(gfx, cfg, right, shift, 35, 95, 49, 97, 49, 99, 35, 97, eye_rgb(224, 127, 68), drop);
		for (int16_t row = 69; row <= 82; row += 6) {
			quad(gfx, cfg, right, shift, 20, row, 23, row + 1, 27, row + 8, 24, row + 6, shade, drop);
		}
		quad(gfx, cfg, right, shift, 38, 84, 39, 84, 39, 92, 38, 92, shell, drop);
	}
}

/** @brief Stagger the individual mechanical assemblies, reversing their order when opening. */
inline void helmet(Arduino_GFX *gfx, const EyeConfig &cfg, float closed, bool breaking) {
	if (closed <= 0) { return; }
	auto travel = [&](float offset) {
		float t = closed * 1.6f - offset;
		t = t < 0 ? 0 : t > 1 ? 1 : t;
		return t * t * (3 - 2 * t);
	};
	visor(gfx, cfg, travel(breaking ? 0.6f : 0));
	cheeks(gfx, cfg, travel(0.3f));
	jaw(gfx, cfg, travel(breaking ? 0 : 0.6f));
}

/** @brief Keep the configured style's real eyes beneath the retracting armor. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float) {
	EyeConfig original = cfg; original.expression = NEUTRAL;
	float closed = closure(elapsed, cfg.expression);
	uint8_t conceal = closed > 0.8f ? uint8_t((closed - 0.8f) * 500) : 0;
	original.fgColor = eye_mix(cfg.bgColor, cfg.fgColor, conceal);
	original.pupilColor = eye_mix(cfg.bgColor, cfg.pupilColor, conceal);
	original.highlightColor = eye_mix(cfg.bgColor, cfg.highlightColor, conceal);
	for (const EyeBox &b : {layout.left, layout.right}) {
		if (closed >= 1) { break; }
		eye_body(gfx, original, b);
		eye_pupils(gfx, original, EyeState{}, b);
	}
	helmet(gfx, cfg, closed,
		cfg.expression == MECHA_BREAK && elapsed % 6000 > 2600);
	return true;
}

/** @brief Register the closed-mask helmet without changing public IDs. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "mecha"; p.fullScene = true;
	p.scene = scene; p.bounds = face_backgrounds::full_bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid mecha face");
} }

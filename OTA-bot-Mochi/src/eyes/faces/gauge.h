/**
 * @file gauge.h
 * @brief gauge definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace gauge_face {
using namespace face_primitives;
inline EyeBox local_eye_gauge_box(const EyeConfig &cfg);
inline float local_eye_gauge_sweep(uint32_t elapsed);
inline void local_eye_gauge_draw(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t now, uint8_t opacity);
inline void local_eye_scene_gauge(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t);


/** @brief gauge local_eye_gauge_box implementation. */
inline EyeBox local_eye_gauge_box(const EyeConfig &cfg) {
	int16_t size = (cfg.screenW < cfg.screenH ? cfg.screenW : cfg.screenH) * 3 / 4;
	return {int16_t((cfg.screenW - size) / 2), int16_t((cfg.screenH - size) / 2), size, size, 0, true};
}

/** @brief gauge local_eye_gauge_sweep implementation. */
inline float local_eye_gauge_sweep(uint32_t elapsed) {
	uint32_t phase = elapsed % 12000;
	if (phase < 2000 || phase >= 10000) { return 0; }
	phase = (phase - 2000) % 4000;
	if (phase < 1600) {
		float t = phase / 1600.0f;
		return t * t * (3 - 2 * t);
	}
	if (phase < 3200) {
		uint32_t beat = (phase - 1600) % 400;
		float cut = beat < 80 ? beat / 80.0f : (400 - beat) / 320.0f;
		return 1 - 0.18f * cut * cut * (3 - 2 * cut);
	}
	float t = (phase - 3200) / 800.0f;
	return 1 - t * t * (3 - 2 * t);
}

/** @brief gauge local_eye_gauge_draw implementation. */
inline void local_eye_gauge_draw(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t now, uint8_t opacity = 100) {
	EyeBox b = local_eye_gauge_box(cfg);
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2, r = b.w / 2 - 3;
	const uint16_t redline = eye_mix(eye_rgb(238, 65, 54), cfg.bgColor, opacity);
	for (uint8_t i = 0; i <= 54; ++i) {
		float angle = 2.3561945f + i * 0.08726646f;
		int16_t outer = r, inner = r - (i % 6 ? 3 : 9);
		uint16_t ink = i >= 42 ? redline : cfg.fgColor;
		gfx->drawLine(int16_t(cx + cosf(angle) * inner), int16_t(cy + sinf(angle) * inner),
			int16_t(cx + cosf(angle) * outer), int16_t(cy + sinf(angle) * outer), ink);
	}
	float sweep = local_eye_gauge_sweep(now);
	float angle = 2.3561945f + sweep * 4.712389f;
	gfx->drawLine(cx, cy, int16_t(cx + cosf(angle) * (r - 14)), int16_t(cy + sinf(angle) * (r - 14)), redline);
	gfx->fillCircle(cx, cy, 3, cfg.fgColor);
}

/** @brief gauge local_eye_scene_gauge implementation. */
inline void local_eye_scene_gauge(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t) {
	EyeBox dial = local_eye_gauge_box(cfg);
	dial.radius = dial.w / 2;
	float reveal = t < 0.65f ? 0 : (t - 0.65f) / 0.35f;
	EyeConfig body = cfg;
	body.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t((1 - reveal) * 100));
	const EyeBox eyes[] = {layout.left, layout.right};
	for (const EyeBox &eye : eyes) {
		EyeBox box = eye_scene_box(eye, dial, t);
		eye_scene_body(gfx, body, box, t);
	}
	if (reveal > 0) {
		gfx->fillCircle(dial.x + dial.w / 2, dial.y + dial.h / 2,
			int16_t((dial.w / 2 - 2) * reveal), cfg.bgColor);
		EyeConfig ink = cfg;
		ink.fgColor = eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(reveal * 100));
		local_eye_gauge_draw(gfx, ink, elapsed, uint8_t(reveal * 100));
	}
}

/** @brief gauge scene procedure. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	local_eye_scene_gauge(gfx, cfg, layout, elapsed, a); return true;
}

/** @brief gauge bounds procedure. */
inline EyeLayout bounds(const EyeConfig &cfg, EyeLayout layout) {

		EyeBox b = local_eye_gauge_box(cfg);
		int16_t x = eye_limit(b.x < layout.left.x ? b.x : layout.left.x, 0, cfg.screenW - 1);
		int16_t y = b.y < layout.left.y ? b.y : layout.left.y;
		if (layout.right.y < y) { y = layout.right.y; }
		int16_t right = b.x + b.w > layout.right.x + layout.right.w ? b.x + b.w : layout.right.x + layout.right.w;
		int16_t bottom = b.y + b.h;
		if (layout.left.y + layout.left.h > bottom) { bottom = layout.left.y + layout.left.h; }
		if (layout.right.y + layout.right.h > bottom) { bottom = layout.right.y + layout.right.h; }
		EyeBox bounds = {x, y, int16_t(right - x), int16_t(bottom - y), 0, true};
		return {bounds, bounds};

}

/** @brief Immutable gauge face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "gauge";
	p.fullScene = true;
	p.scene = scene;
	p.bounds = bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid gauge face parameters");
} }

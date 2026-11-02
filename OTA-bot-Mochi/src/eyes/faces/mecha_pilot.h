/**
 * @file mecha_pilot.h
 * @brief A small rounded pilot helmet with nodding eyes behind a green display.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace mecha_pilot_face {
/** @brief Assemble an original rounded helmet, nod, squint, and reveal the real pair. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout, uint32_t elapsed, float a) {
	float nod = sinf((elapsed % 6000) * 0.0041887902f);
	int16_t w = cfg.screenW * 7 / 10, h = cfg.screenH / 2;
	int16_t x = (cfg.screenW - w) / 2;
	int16_t y = int16_t((cfg.screenH - h) / 2 + a * nod * cfg.screenH / 45);
	int16_t pad = cfg.screenW / 30;
	uint16_t shell = eye_mix(eye_rgb(202, 215, 225), cfg.bgColor, uint8_t(a * 100));
	uint16_t rim = eye_mix(eye_rgb(65, 83, 102), cfg.bgColor, uint8_t(a * 100));
	uint16_t glass = eye_mix(eye_rgb(20, 91, 73), cfg.bgColor, uint8_t(a * 100));
	gfx->fillRoundRect(x, y, w, h, h / 4, shell);
	gfx->fillRoundRect(x + w / 2 - pad, y, 2 * pad, h / 7, pad / 2,
		eye_mix(eye_rgb(202, 80, 86), cfg.bgColor, uint8_t(a * 100)));
	gfx->fillRoundRect(x + pad, y + h / 5, w - 2 * pad, h * 3 / 5, h / 6, rim);
	gfx->fillRoundRect(x + 2 * pad, y + h / 5 + pad, w - 4 * pad, h * 3 / 5 - 2 * pad,
		(h * 3 / 5 - 2 * pad) / 3, glass);
	for (bool right : {false, true}) {
		gfx->fillRoundRect(right ? x + w - pad : x - pad, y + h / 3, 2 * pad, h / 3, pad, rim);
	}
	for (const EyeBox &source : {layout.left, layout.right}) {
		int16_t ew = w / 7;
		float squint = 0.5f + 0.5f * sinf((elapsed % 6000) * 0.0062831853f);
		int16_t eh = int16_t(h * (0.12f + 0.15f * squint));
		int16_t cx = int16_t(x + w * (source.left ? 0.34f : 0.66f));
		EyeBox target{int16_t(cx - ew / 2), int16_t(y + h / 2 - eh / 2), ew, eh,
			int16_t((ew < eh ? ew : eh) / 2), source.left};
		EyeConfig ink = cfg; ink.fgColor = eye_performance_ink(cfg, source.left, elapsed);
		eye_scene_body(gfx, ink, eye_scene_box(source, target, a), a);
	}
	return true;
}

/** @brief Add a pilot performance instead of another variant of the large angular armor. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "mecha_pilot"; p.performance = p.fullScene = true;
	p.performanceInk = eye_rgb(164, 246, 190); p.performanceScene = scene;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid mecha pilot face");
} }

/**
 * @file face_scene_regressions.h
 * @brief Scene colors, deterministic variation and positive-stroke silhouette regressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/eyes/faces/snickering.h"
#include "../OTA-bot-Mochi/src/eyes/faces/hollow_anger.h"
#include <cassert>

/** @brief Dragon remains red and gold on all styles, without changing configured eye ink. */
inline void dragon_colors() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		eyes_init(&gfx);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_set_style(EyeStyle(style));
			EyeConfig cfg = eyes_config(); cfg.expression = CHINESE_DRAGON; cfg.bgColor = 0x0841;
			gfx.fillScreen(cfg.bgColor);
			auto p = face_preset(CHINESE_DRAGON);
			assert(p && p->performanceScene);
			assert(p->performanceScene(&gfx, cfg, eye_layout(cfg, EyeState{}), 3000, 1));
			for (uint16_t ink : {eye_rgb(226, 51, 43), eye_rgb(244, 193, 89)}) {
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), ink) > 40);
			}
			assert(cfg.fgColor == eyes_config().fgColor && eyes_config().expression != CHINESE_DRAGON);
		}
	}
}

/** @brief Varied scenery is reproducible, differs across cycles and eases continuously between samples. */
inline void varied_weather() {
	Arduino_GFX gfx(240, 280);
	eyes_init(&gfx);
	for (Expression face : {RAIN, SMOKE, STARGAZING, BUZZING, ENERGETIC}) {
		EyeConfig cfg = eyes_config(); cfg.expression = face;
		const FacePreset *p = face_preset(face); assert(p && (p->scenery || p->performanceScene));
		EyeLayout pair = eye_layout(cfg, EyeState{});
		auto render = [&](uint32_t time) {
			gfx.fillScreen(0);
			if (p->scenery) { p->scenery(&gfx, cfg, pair, time, 1); }
			else { assert(p->performanceScene(&gfx, cfg, pair, time, 1)); }
		};
		render(2250);
		auto first = gfx.pixels;
		render(8250);
		assert(first != gfx.pixels);
		render(2250);
		assert(first == gfx.pixels);
	}
	for (uint32_t t = 1; t < 5000; ++t) {
		float value = face_smooth_random(t, 1100, 173);
		assert(value >= 0 && value <= 1);
		assert(fabsf(value - face_smooth_random(t - 1, 1100, 173)) < 0.002f);
	}
	auto p = face_preset(DISTRACTED); assert(p && p->layout);
	EyeConfig cfg = eyes_config(); EyeBox first{20, 100, 50, 50, 8, true}, next = first, again = first;
	p->layout(cfg, first, 2700, 1); p->layout(cfg, next, 8700, 1); p->layout(cfg, again, 2700, 1);
	assert(first.x != next.x || first.y != next.y);
	assert(first.x == again.x && first.y == again.y);
}

/** @brief Every full-laugh row contains only a narrow positive stroke, mirrored between eyes. */
inline void snickering_strokes() {
	for (uint8_t style = 0; style < 4; ++style) {
		Arduino_GFX left(80, 80), right(80, 80);
		EyeConfig cfg{}; cfg.style = EyeStyle(style); cfg.fgColor = 0xFFFF;
		EyeBox box{10, 10, 48, 36, 8, true};
		face_defs::snickering_face::eye(&left, cfg, box, 1);
		box.left = false;
		face_defs::snickering_face::eye(&right, cfg, box, 1);
		for (int row = 0; row < box.h; ++row) {
			unsigned count = 0;
			for (int col = 0; col < box.w; ++col) {
				uint16_t pixel = left.pixels[size_t(box.y + row) * 80 + box.x + col];
				count += pixel != 0;
				assert(pixel == right.pixels[size_t(box.y + row) * 80 + box.x + box.w - 1 - col]);
			}
			assert(count == unsigned(box.w / 8));
		}
	}
}

/** @brief Exactly one anger marker belongs on the right, never over the central forehead. */
inline void lateral_anger_markers() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		eyes_init(&gfx);
		EyeConfig cfg = eyes_config(); cfg.expression = HOLLOW_ANGER;
		auto p = face_preset(HOLLOW_ANGER);
		assert(p->performanceScene(&gfx, cfg, eye_layout(cfg, EyeState{}), 3000, 1));
		uint16_t red = eye_mix(eye_rgb(245, 102, 86), cfg.bgColor, 85);
		unsigned left = 0, right = 0;
		for (size_t i = 0; i < gfx.pixels.size(); ++i) {
			if (gfx.pixels[i] != red) { continue; }
			int x = int(i % cfg.screenW);
			assert(x < cfg.screenW / 3 || x > cfg.screenW * 2 / 3);
			if (x < cfg.screenW / 3) { ++left; } else { ++right; }
		}
		assert(left == 0 && right > 0);
	}
}

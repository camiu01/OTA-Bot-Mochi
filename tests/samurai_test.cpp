/**
 * @file samurai_test.cpp
 * @brief Samurai asymmetry, eye slope, broad blade and moving highlight on all panels/styles.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/faces/samurai.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <iostream>
namespace samurai = face_defs::samurai_face;

/** @brief The fading blade must never erase any part of either eye during entry or return. */
static void no_blade_cuts(const EyeConfig &cfg) {
	Arduino_GFX expected(cfg.screenW, cfg.screenH), actual(cfg.screenW, cfg.screenH);
	for (uint32_t elapsed = 625; elapsed <= 5375; elapsed += 125) {
		float a = eye_performance_amount(elapsed);
		EyeLayout pair = eye_layout(cfg, EyeState{});
		samurai::layout(cfg, pair.left, elapsed, a); samurai::layout(cfg, pair.right, elapsed, a);
		expected.fillScreen(cfg.bgColor); actual.fillScreen(cfg.bgColor);
		for (const EyeBox &b : {pair.left, pair.right}) { samurai::eye(&expected, cfg, b, elapsed, a); }
		assert(samurai::scene(&actual, cfg, pair, elapsed, a));
		for (size_t pixel = 0; pixel < expected.pixels.size(); ++pixel) {
			if (expected.pixels[pixel] != cfg.bgColor) { assert(actual.pixels[pixel] == expected.pixels[pixel]); }
		}
	}
}

/** @brief Check geometry and foreground blade independently of palette and style. */
static void check(int16_t width, int16_t height, EyeStyle style) {
	Arduino_GFX gfx(width, height);
	eyes_init(&gfx); eyes_set_style(style); eyes_life_reset();
	eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0);
	eyes_set_expression(SAMURAI);
	EyeConfig cfg = eyes_config();
	no_blade_cuts(cfg);
	EyeLayout pair = eye_layout(cfg, EyeState{});
	samurai::layout(cfg, pair.left, 3000, 1); samurai::layout(cfg, pair.right, 3000, 1);
	assert(pair.left.w < pair.right.w && pair.left.h < pair.right.h);
	assert(pair.left.y > pair.right.y);
	assert(samurai::blade_top(cfg, width / 8) > samurai::blade_top(cfg, width * 7 / 8));
	eyes_state() = EyeState{}; eyes_draw();
	auto first = gfx.pixels;
	eyes_state().lastMs = 3000; eyes_draw();
	auto performed = gfx.pixels;
	assert(performed != first);
	for (int16_t x : {int16_t(width / 4), int16_t(width / 2), int16_t(width * 3 / 4)}) {
		for (int16_t y = 1; y < height / 22; ++y) {
			assert(gfx.pixels[(samurai::blade_top(cfg, x) + y) * width + x] != cfg.bgColor);
		}
	}
	eyes_state().lastMs = 3300; eyes_draw();
	assert(gfx.pixels != performed);
	auto moved = gfx.pixels;
	eyes_state().dirtyInit = false; eyes_draw(); assert(moved == gfx.pixels);
	eyes_state().lastMs = 6000; eyes_draw(); assert(gfx.pixels == first);
	assert(eyes_config().fgColor == 0xFFFF && eyes_config().bgColor == 0x0841);
}
int main() {
	assert(settings_init());
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	for (const auto &p : panels) {
		for (uint8_t style = 0; style < 4; ++style) { check(p[0], p[1], EyeStyle(style)); }
	}
	std::cout << "Samurai asymmetric eyes, diagonal blade, moving glint and clean return passed on six panels\n";
}

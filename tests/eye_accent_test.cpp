/**
 * @file eye_accent_test.cpp
 * @brief Resting faces retain configured ink alongside explicit themed symbol accents.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/eyes/face_support.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <utility>
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"

/** @brief Accept only the explicit semantic accents owned by a symbol face. */
static bool themed_pixel(Expression face, uint16_t pixel) {
	switch (face) {
		case LOVE: return pixel == eye_rgb(255, 92, 150);
		case MONEY: return pixel == eye_rgb(48, 224, 112);
		case STAR: case BOLT: case SPARKLE: return pixel == eye_rgb(255, 210, 48);
		case TARGET: case LOW_BATT: return pixel == eye_rgb(255, 88, 96);
		case MUSIC: return pixel == eye_rgb(184, 96, 248);
		case MOON: return pixel == eye_rgb(255, 228, 144);
		case FIRE: return pixel == eye_rgb(255, 112, 32) || pixel == eye_rgb(255, 224, 64);
		case GLITCH: return pixel == eye_rgb(64, 212, 255) || pixel == eye_rgb(248, 64, 184);
		case LOADING:
			if (pixel == eye_rgb(64, 212, 255)) { return true; }
			for (uint8_t age = 1; age < 8; ++age) {
				if (pixel == eye_mix(eye_rgb(64, 212, 255), 0, uint8_t(20 + (7 - age) * 8))) { return true; }
			}
			return false;
		default: return false;
	}
}

/** @brief Resting expressions must retain the selected ink before a reaction starts. */
static void palette_tests() {
	Arduino_GFX gfx;
	for (uint16_t ink : {uint16_t(0xFFFF), uint16_t(0x07FF), uint16_t(0xFD20)}) {
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_init(&gfx);
			eyes_set_style(static_cast<EyeStyle>(style));
			eyes_set_eye_colors(0, ink, 0xF800, 0x07E0);
			for (uint8_t value = 0; value <= CLOCK_EYES; ++value) {
				eyes_set_expression(static_cast<Expression>(value));
				eyes_draw();
				bool solid = style == MOCHI || style == TRAPEZ;
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), ink) > 0);
				for (uint16_t pixel : gfx.pixels) {
					assert(pixel == 0 || pixel == ink || themed_pixel(Expression(value), pixel) ||
						(!solid && (pixel == 0xF800 || pixel == 0x07E0)));
				}
				assert(eyes_config().fgColor == ink);
				if (value == MONEY) { assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), eye_rgb(48, 224, 112)) > 60); }
				if (value == LOVE || value == STAR || value == BOLT) {
					uint16_t accent = value == LOVE ? eye_rgb(255, 92, 150) : eye_rgb(255, 210, 48);
					assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), accent) > 20);
				}
				auto dirty = gfx.pixels;
				eyes_state().dirtyInit = false;
				eyes_draw();
				assert(dirty == gfx.pixels);
			}
		}
	}
}

/** @brief Petting hearts use the selected body color and erase without trails. */
static void heart_tests() {
	Arduino_GFX gfx;
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_init(&gfx);
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0x07FF, 0xF800, 0x07E0);
		eyes_set_expression(NEUTRAL);
		eyes_draw();
		auto baseline = gfx.pixels;
		EyeState &state = eyes_state();
		state.heartUntilMs = 40001;
		state.heartX = 120;
		state.heartY = 40;
		eyes_draw();
		assert(gfx.pixels[43 * gfx.width() + 117] == 0x07FF);
		assert(gfx.pixels != baseline);
		auto dirty = gfx.pixels;
		state.dirtyInit = false;
		eyes_draw();
		assert(dirty == gfx.pixels);
		state.heartUntilMs = 0;
		eyes_draw();
		assert(gfx.pixels == baseline);
	}
}

/** @brief Symbol hooks stay open and thick strokes remain inside their advertised bounds. */
static void symbol_tests() {
	Arduino_GFX gfx(80, 80);
	for (auto draw : {face_primitives::question, face_primitives::money, face_primitives::power_sym, face_primitives::spiral}) {
		gfx.fillScreen(0);
		draw(&gfx, 40, 40, 20, 0xFFFF);
		assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xFFFF) > 90);
		for (int y = 0; y < 80; ++y) {
			for (int x = 0; x < 80; ++x) {
				if (x < 20 || x > 60 || y < 20 || y > 60) { assert(gfx.pixels[y * 80 + x] == 0); }
			}
		}
	}
	gfx.fillScreen(0);
	face_primitives::power_sym(&gfx, 40, 40, 20, 0xFFFF);
	assert(gfx.pixels[27 * 80 + 35] == 0);
	gfx.fillScreen(0);
	face_primitives::question(&gfx, 40, 40, 20, 0xFFFF);
	assert(gfx.pixels[37 * 80 + 30] == 0);
}

/** @brief Spinner and digital tears animate deterministically, erase cleanly and survive timer rollover. */
static void animated_symbols() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_init(&gfx);
			eyes_set_style(EyeStyle(style));
			for (Expression face : {LOADING, GLITCH}) {
				eyes_set_expression(face);
				std::vector<uint16_t> initial;
				bool changed = false;
				for (uint32_t time : {0u, 120u, 720u, 1440u, 1680u, 2400u, 3000u, UINT32_MAX - 1, 0u}) {
					eyes_state().lastMs = time;
					eyes_draw();
					auto dirty = gfx.pixels;
					if (initial.empty()) { initial = dirty; }
					else { changed |= initial != dirty; }
					eyes_state().dirtyInit = false;
					eyes_draw();
					assert(dirty == gfx.pixels);
				}
				assert(changed);
			}
		}
	}
}

/** @brief Colored symbols occupy a meaningful area on both shipped panel sizes. */
static void symbol_coverage() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_init(&gfx); eyes_set_style(EyeStyle(style));
			for (Expression face : {TARGET, SPARKLE, MUSIC, MOON, FIRE, LOW_BATT}) {
				eyes_set_expression(face); eyes_draw();
				eyes_state().lastMs = eyes_state().effectStartMs + 3000; eyes_draw();
				size_t painted = 0;
				for (uint16_t pixel : gfx.pixels) { painted += themed_pixel(face, pixel); }
				assert(painted >= (face == LOW_BATT ? 10 : 40));
			}
		}
	}
}

/** @brief Each selected symbol starts and ends pixel-identical to the style's ordinary eyes. */
static void cycle_case(Arduino_GFX &gfx, Expression face, uint32_t entry, uint16_t bg) {
	eyes_life_reset();
	eyes_state() = EyeState{};
	eyes_set_eye_colors(bg, 0xFFFF, 0xF800, 0x07E0);
	eyes_set_expression(NEUTRAL);
	eyes_state().lastMs = entry;
	eyes_draw();
	auto normal = gfx.pixels;
	eyes_set_expression(face);
	for (uint32_t elapsed : {0u, 300u, 600u, 800u, 1049u, 1050u, 1051u, 1200u, 1500u,
		2400u, 3000u, 4500u, 4800u, 4949u, 4950u, 4951u, 5100u, 5400u, 5999u, 6000u}) {
		eyes_state().lastMs = entry + elapsed;
		eyes_draw();
		auto dirty = gfx.pixels;
		if (elapsed <= 600 || elapsed >= 5400) { assert(dirty == normal); }
		if (elapsed == 3000) { assert(dirty != normal); }
		eyes_state().dirtyInit = false; eyes_draw();
		assert(gfx.pixels == dirty);
		assert(eyes_config().expression == face && eyes_config().fgColor == 0xFFFF);
	}
}

/** @brief Reversible symbol handoffs cover all styles, six panels, custom backgrounds and time rollover. */
static void normal_cycle_tests() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}, {240, 320}, {320, 480}, {128, 160}, {160, 128}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		eyes_init(&gfx);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_set_style(EyeStyle(style));
			for (Expression face : {TARGET, SPARKLE, MUSIC, MOON, LOW_BATT, LOADING, GLITCH, FIRE}) {
				cycle_case(gfx, face, 0, 0);
				cycle_case(gfx, face, UINT32_MAX - 300, 0x0841);
			}
		}
	}
}

/** @brief Run configured-palette and marker regressions. @return Success. */
int main() {
	assert(settings_init());
	symbol_tests();
	symbol_coverage();
	animated_symbols();
	normal_cycle_tests();
	palette_tests();
	heart_tests();
	std::cout << "Configured expression palette and petting marker passed\n";
	return 0;
}

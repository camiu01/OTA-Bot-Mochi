/**
 * @file eye_accent_test.cpp
 * @brief Fixed semantic accents: palette safety and visible paint on every style.
 *
 * An accent that equals a configured ink colour would either vanish into the
 * eye or break the solid-style pupil/highlight exclusion the other suites count,
 * and an accent that never reaches the raster is a silently dead decoration.
 * This suite proves the palette stays clear of those values, that every
 * accented expression really paints its accent on all four styles, that the
 * plain faces never leak one, and that the petting heart marker uses the LOVE
 * accent.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/eyes/eye_palette.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <vector>

/** @brief Collect every distinct non-zero accent exactly once. @return Accent colours. */
static std::vector<uint16_t> palette() {
	std::vector<uint16_t> colors;
	for (uint8_t value = 0; value < EXPRESSION_COUNT; ++value) {
		uint16_t accent = eye_accent(static_cast<Expression>(value));
		if (accent && std::find(colors.begin(), colors.end(), accent) == colors.end()) {
			colors.push_back(accent);
		}
	}
	return colors;
}

/** @brief Accents must be many, distinct, and clear of the configured test ink. */
static void palette_tests() {
	assert(eye_accent(NEUTRAL) == 0);
	std::vector<uint16_t> colors = palette();
	assert(colors.size() >= 12);
	for (size_t i = 0; i < colors.size(); ++i) {
		assert(colors[i] != 0x0000 && colors[i] != 0xFFFF);
		assert(colors[i] != 0xF800 && colors[i] != 0x07E0);
		for (size_t j = i + 1; j < colors.size(); ++j) { assert(colors[i] != colors[j]); }
	}
}

/** @brief Plain faces must not paint a single accent pixel. */
static void plain_face_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	std::vector<uint16_t> colors = palette();
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		EyeState &state = eyes_state();
		state.saccadeX = state.saccadeY = state.tiltX = state.tiltY = 0;
		state.blinking = false;
		state.heartUntilMs = 0;
		eyes_set_expression(NEUTRAL);
		state.dirtyInit = false;
		eyes_draw();
		for (uint16_t accent : colors) {
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), accent) == 0);
		}
	}
}

/** @brief Every accented expression must reach the raster on all four styles. */
static void accent_paint_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		bool solid = style == MOCHI || style == TRAPEZ;
		for (uint8_t value = 0; value < EXPRESSION_COUNT; ++value) {
			Expression expr = static_cast<Expression>(value);
			uint16_t accent = eye_accent(expr);
			if (!accent) { continue; }
			EyeState &state = eyes_state();
			state.saccadeX = state.saccadeY = state.tiltX = state.tiltY = 0;
			state.blinking = false;
			state.heartUntilMs = 0;
			eyes_set_expression(expr);
			state.dirtyInit = false;
			eyes_draw();
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), accent) > 0);
			if (solid) {
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xF800) == 0);
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0x07E0) == 0);
			}
		}
	}
}

/** @brief The petting heart marker paints the LOVE accent on every style. */
static void heart_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_expression(NEUTRAL);
		EyeState &state = eyes_state();
		state.dirtyInit = false;
		eyes_draw();
		state.heartUntilMs = 40001;
		state.heartX = 120;
		state.heartY = 60;
		eyes_draw();
		assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), eye_accent(LOVE)) > 0);
		state.heartUntilMs = 0;
	}
}

/** @brief Semantic mood bodies reach all styles without changing the saved palette or leaving trails. */
static void mood_color_tests() {
	Arduino_GFX gfx;
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_init(&gfx); eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		for (Expression mood : {HAPPY, SAD, ANGRY, SLEEPY, SHY, FOCUSED}) {
			uint16_t color = eye_mood_color(mood, 0xFFFF);
			assert(color && color != 0xFFFF && color != 0xF800 && color != 0x07E0);
			eyes_set_expression(mood);
			eyes_draw();
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), color) > 0);
			assert(eyes_config().fgColor == 0xFFFF);
			auto dirty = gfx.pixels;
			eyes_state().dirtyInit = false; eyes_draw();
			assert(dirty == gfx.pixels);
		}
		eyes_set_expression(NEUTRAL); eyes_draw();
		assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), eye_mood_color(FOCUSED, 0xFFFF)) == 0);
		assert(eye_mood_color(NEUTRAL, 0xFFFF) == 0xFFFF);
	}
}

/** @brief Run palette, paint, and marker regressions. @return Success. */
int main() {
	assert(settings_init());
	palette_tests();
	plain_face_tests();
	accent_paint_tests();
	heart_tests();
	mood_color_tests();
	std::cout << "Expression accent palette, per-style paint, and petting marker passed\n";
	return 0;
}

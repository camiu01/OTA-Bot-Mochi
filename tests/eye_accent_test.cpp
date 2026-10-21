/**
 * @file eye_accent_test.cpp
 * @brief Resting faces and petting markers keep the configured palette.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

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
					assert(pixel == 0 || pixel == ink ||
						(!solid && (pixel == 0xF800 || pixel == 0x07E0)));
				}
				assert(eyes_config().fgColor == ink);
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

/** @brief Run configured-palette and marker regressions. @return Success. */
int main() {
	assert(settings_init());
	palette_tests();
	heart_tests();
	std::cout << "Configured expression palette and petting marker passed\n";
	return 0;
}

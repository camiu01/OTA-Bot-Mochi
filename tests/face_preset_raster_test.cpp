/**
 * @file face_preset_raster_test.cpp
 * @brief Frozen pre-preset pixel/dirty-bound fingerprints, not a second renderer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#ifndef RECORD_FACE_BASELINE
#ifdef ALL_FACES_RASTER
#include "fixtures/all_faces_raster.h"
#else
#include "fixtures/face_raster.h"
#endif
#endif

/** @brief Hash fixed-width words in a platform-independent order. */
static void word(uint64_t &hash, uint16_t value) {
	for (unsigned shift : {0u, 8u}) {
		hash ^= (value >> shift) & 255;
		hash *= UINT64_C(1099511628211);
	}
}

/** @brief Exercise consecutive dirty frames and compare each to a full redraw. */
static uint64_t fingerprint(Arduino_GFX &gfx, Expression expression) {
	eyes_life_reset();
	eyes_set_expression(expression);
	EyeState &st = eyes_state();
	st = EyeState{};
	const bool migrated = expression == TIRED || expression == FOCUSED || expression == SLEEPY_EYES;
#ifdef ALL_FACES_RASTER
	(void)migrated;
	const unsigned samples = 16;
#else
	const unsigned samples = migrated ? 16 : 2;
#endif
	uint64_t hash = UINT64_C(14695981039346656037);
	for (unsigned sample = 0; sample < samples; ++sample) {
		eyes_set_asym(sample & 1);
		eyes_set_eye_colors(sample & 8 ? 0x0841 : 0,
			sample & 8 ? 0x07FF : 0xFFFF, 0xF800, 0x07E0);
		st.lastMs = sample & 1 ? 3000 : 0;
#ifdef ALL_FACES_RASTER
		st.lastMs = sample * 1000;
#endif
		st.blinking = sample & 2;
		st.saccadeX = sample & 4 ? -17 : 13;
		st.saccadeY = sample & 4 ? 11 : -9;
		st.tiltX = sample & 4 ? 8 : -6;
		st.tiltY = 3; st.floatY = -2;
		if (sample == 8) { st.dirtyInit = false; } // Palette changes require a full clear.
		eyes_draw();
		for (uint16_t pixel : gfx.pixels) { word(hash, pixel); }
		word(hash, st.prevX); word(hash, st.prevY);
		word(hash, st.prevW); word(hash, st.prevH);
		auto dirty = gfx.pixels;
		st.dirtyInit = false;
		eyes_draw();
		if (dirty != gfx.pixels) {
			std::cerr << "Dirty/full mismatch: " << gfx.width() << "x" << gfx.height()
				<< " style " << unsigned(eyes_config().style) << " "
				<< eyes_expression_name(expression) << " sample " << sample << "\n";
			assert(false);
		}
	}
	return hash;
}

/** @brief Compare all expressions on the six supported native panel fixtures. */
int main(int argc, char **argv) {
	assert(settings_init());
#ifdef RECORD_FACE_BASELINE
	assert(argc == 2);
	std::ofstream output(argv[1]);
	assert(output);
	output << "// Captured from the interrupted workspace before FacePreset migration.\n"
		<< "// FNV-1a of RGB565 pixels and dirty bounds; see face_preset_raster_test.cpp.\n"
		<< "#pragma once\n#include <stdint.h>\n"
		<< "static const uint64_t FACE_RASTER_BASELINE[] = {\n";
#else
	(void)argc; (void)argv;
	size_t index = 0;
#endif
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	for (const auto &panel : panels) {
		Arduino_GFX gfx(panel[0], panel[1]);
		eyes_init(&gfx);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_set_style(EyeStyle(style));
			for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) {
				uint64_t actual = fingerprint(gfx, Expression(id));
#ifdef RECORD_FACE_BASELINE
				output << "UINT64_C(" << actual << "),\n";
#else
				if (actual != FACE_RASTER_BASELINE[index++]) {
					std::cerr << panel[0] << "x" << panel[1] << " style "
						<< unsigned(style) << " " << eyes_expression_name(Expression(id)) << "\n";
					return 1;
				}
#endif
			}
		}
	}
#ifdef RECORD_FACE_BASELINE
	output << "};\n";
	assert(output.good());
#else
	assert(index == sizeof(FACE_RASTER_BASELINE) / sizeof(FACE_RASTER_BASELINE[0]));
	std::cout << "Pre-preset pixel and dirty-bound fingerprints passed on six panels\n";
#endif
}

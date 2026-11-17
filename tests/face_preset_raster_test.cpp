/**
 * @file face_preset_raster_test.cpp
 * @brief Frozen pre-preset pixel/dirty-bound fingerprints, not a second renderer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/face_support.h"
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

/** @brief Check registered spinner pixels against high-precision evaluation of the legacy float angles. */
static void loading_octants() {
	Arduino_GFX actual(240, 280), expected(240, 280);
	EyeConfig cfg{}; cfg.expression = LOADING;
	for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
		cfg.style = EyeStyle(style);
		for (int16_t size = 6; size <= 150; ++size) {
			EyeBox box{7, 19, size, size, int16_t(size / 4), true};
			int16_t half = size / 2, dot = eye_limit(int16_t(half / 7), 1, 6);
			int16_t radius = half - dot - 3, x = box.x + half, y = box.y + half;
			for (uint32_t head = 0; head < 8; ++head) {
				cfg.bgColor = head & 1 ? 0x0841 : 0; cfg.fgColor = 0xFFFF;
				actual.fillScreen(cfg.bgColor); expected.fillScreen(cfg.bgColor);
				EyeState state{}; state.lastMs = head * 180;
				assert(face_preset(LOADING)->referenceBody(&actual, cfg, state, box, 0, cfg.fgColor));
				eye_body(&expected, cfg, box);
				if (radius >= 3) {
					expected.fillCircle(x, y, radius + dot + 1, cfg.bgColor);
					for (unsigned i = 0; i < 8; ++i) {
						double angle = float(i * 0.78539816f);
						float sine = float(std::sin(angle)), cosine = float(std::cos(angle));
						unsigned age = (head + 8 - i) % 8;
						uint8_t amount = age ? uint8_t(20 + (7 - age) * 8) : 100;
						expected.fillCircle(int16_t(x + sine * radius), int16_t(y - cosine * radius),
							dot, eye_mix(eye_rgb(64, 212, 255), cfg.bgColor, amount));
					}
				}
				assert(actual.pixels == expected.pixels);
			}
		}
	}
}

/** @brief Battery glyphs keep valid dimensions while a blink shrinks the charge interior away. */
static void low_battery_sizes() {
	Arduino_GFX gfx(40, 40);
	for (int16_t s = 0; s <= 64; ++s) {
		gfx.fillScreen(0);
		face_primitives::low_batt(&gfx, 20, 20, s, 0xFFFF);
		if (s == 4) { assert(gfx.pixels[20 * 40 + 20] == 0); }
		if (s == 8) { assert(gfx.pixels[19 * 40 + 16] == 0xFFFF); }
	}
}

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
	low_battery_sizes();
	loading_octants();
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
			// Frozen fixtures cover the original IDs only; new moods have cycle tests.
			for (uint8_t id = 0; id <= ENRAGED; ++id) {
				uint64_t actual = fingerprint(gfx, Expression(id));
#ifdef RECORD_FACE_BASELINE
				output << "UINT64_C(" << actual << "),\n";
#else
				const uint64_t original = FACE_RASTER_BASELINE[index++];
				// Every original ID is frozen; deliberate changes update hashes and the fixture history note.
				if (actual != original) {
					std::cerr << panel[0] << "x" << panel[1] << " style "
						<< unsigned(style) << " " << eyes_expression_name(Expression(id))
						<< " expected " << original << " actual " << actual << "\n";
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
	std::cout << "Unchanged face fingerprints and all dirty/full redraws passed on six panels\n";
#endif
}

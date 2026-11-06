/**
 * @file symbol_color_regressions.h
 * @brief Semantic symbol interiors preserve configured bodies and open negative space.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief Each repaired symbol paints its theme on both panels, all styles and custom palettes. */
static void repaired_symbol_colors() {
	const Expression faces[] = {DIAMOND, DEVIL, IDEA, HALO, RING, QUESTION, POWER, SWIRLY};
	const uint16_t colors[] = {eye_rgb(64, 212, 255), eye_rgb(255, 88, 96),
		eye_rgb(255, 210, 48), eye_rgb(255, 210, 48), eye_rgb(64, 212, 255),
		eye_rgb(64, 212, 255), eye_rgb(48, 224, 112), eye_rgb(184, 96, 248)};
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
			eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_life_reset();
			for (uint16_t ink : {uint16_t(0xFFFF), uint16_t(0x07FF), uint16_t(0xFD20)}) {
				for (uint16_t bg : {uint16_t(0), uint16_t(0x0841)}) {
					eyes_set_eye_colors(bg, ink, 0xF800, 0x07E0);
					for (unsigned i = 0; i < 8; ++i) {
						eyes_state() = EyeState{}; eyes_set_expression(faces[i]);
						for (uint32_t time : {0u, 330u, 1500u, 3000u}) {
							eyes_state().lastMs = time; eyes_draw();
							assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), colors[i]) > 0);
							assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), ink) > 0);
							auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw();
							assert(dirty == gfx.pixels && eyes_config().fgColor == ink);
						}
						eyes_set_expression(NEUTRAL); eyes_draw();
						assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), colors[i]) == 0);
					}
				}
			}
		}
	}
}

/** @brief Rings keep their center open; Diamond accents remain inside the original silhouette. */
static void symbol_negative_space() {
	Arduino_GFX gfx(160, 160); EyeConfig cfg{}; cfg.fgColor = 0xFFFF;
	EyeBox box{0, 0, 160, 160, 0, true};
	for (Expression face : {RING}) {
		gfx.fillScreen(0x0841);
		assert(face_preset(face)->glyph(&gfx, cfg, box, 80, 80, 30, 0xFFFF, 0x0841));
		assert(gfx.pixels[80 * 160 + 80] == 0x0841);
	}
	for (Expression face : {DIAMOND}) {
		for (int16_t size = 7; size <= 60; ++size) {
			gfx.fillScreen(0);
			face_primitives::rhombus(&gfx, 80, 80, size, 0xFFFF);
			auto shape = gfx.pixels;
			assert(face_preset(face)->glyph(&gfx, cfg, box, 80, 80, size, 0xFFFF, 0));
			for (size_t i = 0; i < shape.size(); ++i) {
				if (!shape[i] && gfx.pixels[i]) {
					std::cerr << "Symbol spill " << eyes_expression_name(face) << " size " << size
						<< " at " << i % 160 << ',' << i / 160 << '\n';
				}
				assert(shape[i] != 0 || gfx.pixels[i] == 0);
			}
		}
	}
}

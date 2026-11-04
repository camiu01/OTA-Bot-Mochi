/**
 * @file face_variation_test.cpp
 * @brief Seeded motion, loop timing and live dirty-redraw regressions for every face.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/face_variation.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <algorithm>
#include <cassert>
#include <iostream>

/** @brief Time never reverses or leaves its cycle; entry/restore windows stay exact. */
static void timing() {
	EyeConfig cfg{};
	for (uint8_t face = 0; face < EXPRESSION_COUNT; ++face) {
		cfg.expression = Expression(face);
		uint32_t duration = face_preset(cfg.expression)->normalCycle ? 6000 : FacePolicy::cycle_duration(cfg.expression);
		for (uint32_t seed : {0u, 1u, 173u, UINT32_MAX}) {
			cfg.variationSeed = seed;
			for (float amount : {0.0f, 0.55f, 1.2f, 10.0f}) {
				uint32_t previous = 0;
				for (uint32_t now = 0; now <= duration * 3; now += 17) {
					uint32_t value = face_variation_elapsed(cfg, now, amount), phase = now % duration;
					assert(value >= previous && value / duration == now / duration);
					assert(value == face_variation_elapsed(cfg, now, amount));
					if (!seed || !amount || phase <= 800 || phase >= duration - 800) { assert(value == now); }
					previous = value;
				}
				for (uint32_t cycle = 0; cycle < 4; ++cycle) {
					assert(face_variation_elapsed(cfg, duration * cycle, amount) == duration * cycle);
				}
				assert(face_variation_elapsed(cfg, UINT32_MAX, amount) >= UINT32_MAX - 1200);
			}
		}
	}
	cfg.expression = RAIN; cfg.variationSeed = 173;
	uint32_t first = face_variation_elapsed(cfg, 3000, 1);
	cfg.variationSeed = 947;
	assert(first != face_variation_elapsed(cfg, 3000, 1));
	assert(face_random(5, 173) == face_random(5, 173, 0));
	assert(face_random(5, 173, 1) != face_random(5, 173, 2));
}

/** @brief Entry seeds remain stable, switch behind the blink and survive timestamp rollover. */
static void motion() {
	EyeLife life;
	EyeConfig cfg{}; cfg.style = MOCHI; cfg.expression = NEUTRAL; cfg.asym = 1;
	EyeState st{};
	uint32_t start = UINT32_MAX - 10000;
	life.step(cfg, st, start, 0, 1, 1, 173);
	assert(life.get().variationSeed == 173);
	cfg.expression = HAPPY;
	life.step(cfg, st, start + 33, 33, 1, 1, 947);
	assert(life.get().variationSeed == 173 && life.get().expression == NEUTRAL);
	life.step(cfg, st, start + 145, 100, 1, 1, 511);
	assert(life.get().variationSeed == 947 && life.get().expression == HAPPY);
	assert(life.get().openL < 0.01f && life.get().openR < 0.01f);
	for (uint32_t t = 400; t < 90000; t += 33) {
		life.step(cfg, st, start + t, 33, 2, 1.2f, t * 733u);
		const auto frame = life.get();
		assert(frame.variationSeed == 947 && frame.expression == HAPPY);
		assert(frame.openL >= 0 && frame.openL <= 1 && frame.openR >= 0 && frame.openR <= 1);
		assert(frame.scaleX > 0.8f && frame.scaleX < 1.2f && frame.scaleY > 0.8f && frame.scaleY < 1.3f);
		assert(abs(frame.x) <= 16 && abs(frame.y) <= 16);
		life.step(cfg, st, start + t, 0, 2, 1.2f, 0);
		assert(life.get().x == frame.x && life.get().scaleX == frame.scaleX);
	}
	life.step(cfg, st, 100000, 33, 0, 1, UINT32_MAX);
	assert(life.get().variationAmount == 0 && life.get().scaleX == 1 && life.get().scaleY == 1);
	assert(life.get().openL == 1 && life.get().openR == 1);
	life.reset(); life.step(cfg, st, 0, 0, 1, 1, UINT32_MAX);
	assert(life.get().variationSeed != 0);
}

/** @brief Redrawing the same live frame must erase old geometry and retain the configured palette. */
static void redraw(Arduino_GFX &gfx) {
	eyes_draw();
	auto dirty = gfx.pixels;
	eyes_state().dirtyInit = false;
	eyes_draw();
	if (dirty != gfx.pixels) {
		std::cerr << "Dirty mismatch: " << eyes_expression_name(eyes_config().expression)
			<< " style " << int(eyes_config().style) << " at " << eyes_state().lastMs
			<< " panel " << gfx.width() << 'x' << gfx.height() << '\n';
		assert(false);
	}
	assert(eyes_config().variationSeed == 0 && eyes_config().fgColor == 0xFFFF);
}

/** @brief Every face/style covers two cycles, transitions and native panel dimensions with live variation. */
static void raster() {
	auto p = settings_get(); p.random_faces = p.imu_faces = p.shake = false;
	p.animation = 2; p.dashboard = false; assert(settings_update(p));
	const int panels[][2] = {{240, 280}, {368, 448}, {240, 320}, {320, 480}, {128, 160}, {160, 128}};
	for (const auto &panel : panels) {
		Arduino_GFX gfx(panel[0], panel[1]);
		for (uint8_t style = 0; style < 7; ++style) {
			eyes_init(&gfx); eyes_set_style(EyeStyle(style));
			eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
			uint32_t base = 0;
			for (uint8_t face = 0; face < EXPRESSION_COUNT; ++face) {
				eyes_set_expression(Expression(face));
				for (uint32_t t : {0u, 112u, 320u, 800u, 1800u, 3000u, 5400u, 6000u, 9500u, 11200u, 12000u, 14300u, 24000u}) {
					test_now = base + t; eyes_state().lastMs = test_now;
					eyes_life_step(test_now, 33); redraw(gfx);
				}
				base += 25000;
			}
		}
	}
}

/** @brief Run deterministic policies and the real raster, without building firmware. */
int main() {
	assert(settings_init());
	timing(); motion(); raster();
	std::cout << "Seeded face motion, monotonic timing and all-face live redraw passed (7 styles, 6 panels)\n";
}

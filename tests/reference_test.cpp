/**
 * @file reference_test.cpp
 * @brief Video-inspired faces, animated dirty rectangles and bounded Pong physics.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/eye_effects.h"
#include "../OTA-bot-Mochi/src/eyes/eye_scenes.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include "../OTA-bot-Mochi/src/play/mini_widgets.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

/** @brief Paddle bounds, bounce, scoring, rollover, and stalled frames remain deterministic. */
static void pong_physics() {
	PongGame game;
	game.reset(0);
	game.move(-100); assert(game.left == 16);
	game.move(999); assert(game.left == 144);
	game.x = 16; game.y = 80; game.vx = -100; game.left = 80;
	game.tick(20, false);
	assert(game.vx > 0 && game.x >= 15);
	game.x = 224; game.vx = 100; game.right = game.y;
	game.tick(40, false);
	assert(game.vx < 0);
	game.y = 3; game.vy = -60;
	game.tick(60, false); assert(game.vy > 0);
	game.y = 157; game.vy = 60;
	game.tick(80, false); assert(game.vy < 0);
	game.x = 4; game.y = 150; game.left = 16; game.vx = -100;
	game.tick(100, false);
	assert(game.opponent == 1 && game.x >= 119 && game.x <= 121);
	game.opponent = 99; game.x = 4; game.y = 150; game.vx = -100;
	game.tick(120, false); assert(game.opponent == 0);
	game.x = 236; game.y = 3; game.right = 144; game.vx = 100;
	game.tick(140, false); assert(game.player == 1);
	game.reset(UINT32_MAX - 10); game.tick(10, true);
	assert(game.x > 120 && game.x < 123);
	float x = game.x;
	game.tick(10, true); assert(game.x == x);
	game.tick(100000, true); assert(game.x - x <= 10.1f);
	game.reset(0);
	for (uint32_t now = 0; now < 120000; now += 33) {
		game.tick(now, true);
		assert(game.x >= 3 && game.x <= 237 && game.y >= 3 && game.y <= 157);
		assert(game.left >= 16 && game.left <= 144 && game.right >= 16 && game.right <= 144);
	}
}

/** @brief New names round-trip through USB and the unchanged v5 profile. */
static void preference_tests() {
	assert(CYBORG == 87 && ARC_HAPPY == 88 && CLOCK_EYES == 93);
	for (uint8_t value = ARC_HAPPY; value < EXPRESSION_COUNT; ++value) {
		CompanionSettings p;
		p.mood = value;
		uint8_t bytes[SETTINGS_BYTES];
		settings_encode(p, bytes);
		CompanionSettings decoded;
		assert(bytes[0] == 5 && SETTINGS_BYTES == 13);
		assert(settings_decode(bytes, sizeof(bytes), decoded) && decoded.mood == value);
		String command = String("mood ") + settings_mood_name(value);
		auto parsed = settings_parse(command.c_str(), p);
		assert(parsed.action == UsbAction::Set && parsed.profile.mood == value);
	}
}

/** @brief Animated scenes erase previous bounds, progress, and never add solid-style pupils. @param width Panel width. @param height Panel height. */
static void animated_faces(int16_t width, int16_t height) {
	Arduino_GFX gfx(width, height);
	eyes_init(&gfx);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		for (Expression expression : {PONG, CLOCK_EYES, GAUGE, GLOW, HEADLIGHTS}) {
			eyes_life_reset();
			eyes_set_expression(expression);
			std::vector<uint16_t> first;
			bool changed = false;
			for (uint32_t now = 0; now <= 12500; now += 250) {
				EyeState &st = eyes_state();
				st.lastMs = now; st.blinking = false;
				st.saccadeX = now % 500 ? -12 : 12;
				eyes_draw();
				auto dirty = gfx.pixels;
				if (first.empty()) { first = dirty; }
				else { changed |= first != dirty; }
				if ((expression == PONG || expression == GAUGE) && (now == 11250 || now == 12000)) {
					assert(dirty == first);
				}
				st.dirtyInit = false; eyes_draw();
				assert(gfx.pixels == dirty);
				assert(st.prevX >= 0 && st.prevY >= 0);
				assert(st.prevX + st.prevW <= width && st.prevY + st.prevH <= height);
				if (style == MOCHI || style == TRAPEZ) {
					assert(std::count(dirty.begin(), dirty.end(), 0xF800) == 0);
					assert(std::count(dirty.begin(), dirty.end(), 0x07E0) == 0);
				}
			}
			assert(changed);
			if (expression == GLOW || expression == HEADLIGHTS || expression == GAUGE) {
				uint16_t saved_ink = eyes_config().fgColor;
				uint8_t saved_color = settings_get().color;
				eyes_state().saccadeX = 0;
				eyes_state().lastMs = 0;
				eyes_draw();
				auto stationary = gfx.pixels;
				eyes_state().lastMs = 1800;
				eyes_draw();
				assert(gfx.pixels != stationary); // Time alone must animate each effect.
				assert(eyes_config().fgColor == saved_ink && settings_get().color == saved_color);
			}
		}
		eyes_set_expression(NEUTRAL);
		eyes_draw();
		auto dirty = gfx.pixels;
		eyes_state().dirtyInit = false; eyes_draw();
		assert(gfx.pixels == dirty);
	}
}

/** @brief Morph endpoints reverse exactly; entry clocks wrap and Glow restores the selected ink. */
static void effect_cycles() {
	assert(eye_scene_amount(0) == 0 && eye_scene_amount(800) == 0);
	assert(eye_scene_amount(2000) == 1 && eye_scene_amount(10000) == 1);
	assert(eye_scene_amount(11200) == 0 && eye_scene_amount(12000) == 0);
	assert(eye_scene_amount(1400) == eye_scene_amount(10600));
	assert(eye_scene_amount(1400) > 0 && eye_scene_amount(1400) < 1);
	for (uint16_t base : {uint16_t(0xFFFF), uint16_t(0x07FF), uint16_t(0x07E0),
			uint16_t(0xFD20), uint16_t(0xF81F), uint16_t(0x001F)}) {
		assert(eye_glow_color(0, base) == base && eye_glow_color(1200, base) == base);
		assert(eye_glow_color(9600, base) == base && eye_glow_color(10000, base) == base);
		assert(eye_glow_color(2400, base) == eye_rgb(255, 112, 190));
		assert(eye_glow_color(8400, base) == eye_rgb(255, 148, 128));
		assert(eye_glow_color(9000, base) == eye_mix(base, eye_rgb(255, 148, 128), 50));
	}
	EyeState st{};
	st.lastMs = UINT32_MAX - 50;
	assert(eye_effect_elapsed(st, GLOW) == 0);
	st.lastMs = 49;
	assert(eye_effect_elapsed(st, GLOW) == 100);
	assert(eye_effect_elapsed(st, GAUGE) == 0);
}

/** @brief Beams start at full eye height and expand outward on both sides. */
static void headlight_origin() {
	EyeConfig cfg{};
	cfg.screenW = 240; cfg.screenH = 280;
	cfg.eyeW = 44; cfg.eyeH = 100; cfg.eyeGap = 42;
	cfg.bgColor = 0; cfg.fgColor = 0xFFFF;
	for (bool left : {true, false}) {
		Arduino_GFX gfx(240, 280);
		EyeBox b = {int16_t(left ? 55 : 141), 90, 44, 100, 20, left};
		int16_t near = left ? b.x - 1 : b.x + b.w;
		int16_t far = left ? 0 : 239;
		eye_headlight(&gfx, cfg, b, 0);
		assert(gfx.paints == 0);
		eye_headlight(&gfx, cfg, b, 600);
		assert(gfx.pixels[140 * 240 + near] != 0);
		assert(gfx.pixels[140 * 240 + far] == 0);
		gfx.fillScreen(0);
		eye_headlight(&gfx, cfg, b, 1200);
		int16_t overlap = left ? b.x + 2 : b.x + b.w - 3;
		assert(gfx.pixels[95 * 240 + overlap] != 0);
		assert(gfx.pixels[91 * 240 + near] != 0);
		assert(gfx.pixels[189 * 240 + near] != 0);
		assert(gfx.pixels[75 * 240 + far] != 0);
		assert(gfx.pixels[205 * 240 + far] != 0);
		assert(gfx.pixels[75 * 240 + near] == 0);
		assert(gfx.pixels[140 * 240 + far] != 0);
	}
}

/** @brief The decorative limiter repeatedly cuts back in the red zone, then recovers. */
static void gauge_limiter() {
	assert(eye_gauge_sweep(2000) == 0);
	assert(eye_gauge_sweep(2800) > 0.49f && eye_gauge_sweep(2800) < 0.51f);
	for (uint32_t lap : {0u, 4000u}) {
		for (uint32_t beat = 0; beat < 4; ++beat) {
			uint32_t peak = 3600 + lap + beat * 400;
			float cut = eye_gauge_sweep(peak + 80);
			assert(eye_gauge_sweep(peak) == 1);
			assert(cut > 0.81f && cut < 0.83f);
			assert(eye_gauge_sweep(peak + 240) > cut);
			assert(eye_gauge_sweep(peak + 400) == 1);
		}
		assert(eye_gauge_sweep(5600 + lap) < eye_gauge_sweep(5200 + lap));
		assert(eye_gauge_sweep(6000 + lap) == 0);
	}
	for (uint32_t t = 0; t < 24000; t += 7) {
		float value = eye_gauge_sweep(t);
		assert(value >= 0 && value <= 1);
		assert(eye_gauge_sweep(t + 12000) == value);
	}
}

/** @brief Automatic reactions wait for real rendered cycles, including after restoring the base. */
static void automatic_cycle_guard() {
	Arduino_GFX gfx(240, 280);
	eyes_init(&gfx);
	CompanionSettings saved = settings_get();
	const Expression moods[] = {GAUGE, PONG, GLOW, HEADLIGHTS, HAPPY};
	const uint32_t lengths[] = {12000, 12000, 9600, 3600, 3000};
	for (uint8_t i = 0; i < 5; ++i) {
		test_now = 50000;
		CompanionSettings p = saved;
		p.mood = uint8_t(moods[i]); p.random_faces = false; p.imu_faces = true; p.shake = false;
		assert(settings_update(p));
		eyes_life_reset(); eyes_update(test_now);
		assert(!eyes_faces_automatic_ready(test_now));
		for (uint32_t t = 50; t < lengths[i]; t += 50) {
			test_now = 50000 + t;
			eyes_faces_motion(0.4f, 0, 0, test_now); eyes_update(test_now);
			assert(eyes_config().expression == moods[i]);
		}
		for (uint32_t t = lengths[i]; t <= lengths[i] + 400; t += 50) {
			test_now = 50000 + t;
			eyes_faces_motion(0.4f, 0, 0, test_now); eyes_update(test_now);
		}
		assert(eyes_config().expression == SKEPTIC);
		for (uint8_t frame = 0; frame < 44; ++frame) {
			test_now += 50; eyes_update(test_now);
		}
		test_now += 400; eyes_update(test_now);
		assert(eyes_config().expression == moods[i]);
		assert(eyes_state().effectExpression == moods[i]);
		assert(!eyes_faces_automatic_ready(test_now));
		uint32_t entry = eyes_state().effectStartMs;
		assert(!eyes_faces_automatic_ready(entry + lengths[i] - 1));
		assert(eyes_faces_automatic_ready(entry + lengths[i]));
	}
	FacePolicy random;
	random.reset(GAUGE, 0);
	assert(random.update(12000, true, false, 0, false) == GAUGE);
	Expression reaction = random.update(12500, true, false, 0, true);
	assert(reaction != GAUGE);
	assert(random.update(12501, true, false, 0, false) == reaction);
	assert(random.update(15500, true, false, 0, false) == GAUGE);
	assert(settings_update(saved));
}

/** @brief Run shared model, profile and panel scene checks. @return Success. */
int main() {
	assert(settings_init());
	pong_physics();
	preference_tests();
	headlight_origin();
	effect_cycles();
	gauge_limiter();
	automatic_cycle_guard();
	assert(eye_mix(0xFFFF, 0x0841, 0) == 0x0841);
	assert(eye_mix(0xFFFF, 0x0841, 100) == 0xFFFF);
	animated_faces(240, 280);
	animated_faces(368, 448);
	animated_faces(128, 160);
	std::cout << "Reference faces, animated scenes, v5 profiles, and Pong physics passed\n";
}

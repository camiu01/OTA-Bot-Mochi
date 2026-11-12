/**
 * @file animated_emotions_test.cpp
 * @brief Emotion-specific motion, brief glitch recovery, contained blush and clean live redraw.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/face_support.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/faces/mecha_break.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <Preferences.h>
#include <array>
#include <cassert>
#include <iostream>
#include "cinematic_face_checks.h"

/** @brief Sample the actual registered motion without idle motion or blink interference. */
static std::array<float, 4> motion(Expression expression, uint32_t time, float amount = 1) {
	std::array<float, 4> pose{1, 1, 0, 0};
	auto preset = face_preset(expression); assert(preset && preset->reactionMotion);
	preset->reactionMotion(true, time, amount, pose[0], pose[1], pose[2], pose[3]);
	return pose;
}

/** @brief Independent intent checks: curiosity lifts, suspicion narrows, shyness dips, sleep nods/wakes. */
static void gestures() {
	auto curious = motion(CURIOUS, 750);
	assert(curious[1] > 1 && curious[2] > 4 && curious[3] < 0);
	assert(motion(CURIOUS, 2250)[2] < -4);
	auto suspicious = motion(SUSPICIOUS, 750);
	assert(suspicious[1] < .9f && suspicious[2] > 7 && suspicious[3] == 0);
	assert(motion(SUSPICIOUS, 2250)[2] < -7);
	auto shy = motion(SHY, 750);
	assert(shy[2] > 3 && shy[3] > 0);
	assert(motion(SLEEPY, 1500)[1] < .6f && motion(SLEEPY, 1500)[3] >= 6);
	assert(motion(SLEEPY, 2450)[1] > 1 && motion(SLEEPY, 2450)[3] < 1);
	auto first = motion(LOVE, 120), second = motion(LOVE, 440), rest = motion(LOVE, 900);
	assert(first[0] > second[0] && second[0] > rest[0] && rest[0] == 1);
	assert(first[1] > 1 && first[3] < 0);
	for (Expression face : {SLEEPY, CURIOUS, SHY, SUSPICIOUS, LOVE}) {
		auto end = motion(face, 3000), start = motion(face, 0);
		for (unsigned i = 0; i < 4; ++i) { assert(std::abs(start[i] - end[i]) < .0001f); }
		assert((motion(face, 1500, 0) == std::array<float, 4>{1, 1, 0, 0}));
	}
}

/** @brief Glitch is fully intact between brief bursts, with no fragment outside the eye box. */
static void glitch_recovery() {
	Arduino_GFX gfx(240, 280);
	EyeConfig cfg{}; cfg.style = MOCHI; cfg.expression = GLITCH; cfg.fgColor = 0xFFFF;
	EyeBox box{50, 80, 60, 90, 12, true};
	gfx.fillScreen(0); eye_body(&gfx, cfg, box); auto intact = gfx.pixels;
	for (uint32_t seed : {0u, 173u, 947u}) {
		cfg.variationSeed = seed; unsigned corrupted = 0;
		for (uint32_t time = 0; time < 3600; time += 10) {
			EyeState state{}; state.lastMs = time; gfx.fillScreen(0);
			assert(face_preset(GLITCH)->referenceBody(&gfx, cfg, state, box, 0, cfg.fgColor));
			bool changed = gfx.pixels != intact;
			if (time >= 3000) { assert(!changed); }
			corrupted += changed;
			for (int y = 0; y < gfx.h; ++y) {
				for (int x = 0; x < gfx.w; ++x) {
					if (x < box.x || x >= box.x + box.w || y < box.y || y >= box.y + box.h) {
						assert(gfx.pixels[y * gfx.w + x] == 0);
					}
				}
			}
		}
		assert(corrupted > 0 && corrupted < 60); // Brief interruptions, not a permanently broken face.
	}
}

/** @brief Pink hatches appear only temporarily inside the shy eye's lower lid. */
static void blush() {
	Arduino_GFX gfx(240, 280);
	EyeConfig cfg{}; cfg.style = MOCHI; cfg.expression = SHY; cfg.fgColor = 0xFFFF;
	EyeBox box{50, 80, 60, 60, 12, true};
	for (uint32_t time : {0u, 1500u, 3000u}) {
		gfx.fillScreen(0);
		assert(face_preset(SHY)->reactionBody(&gfx, cfg, EyeState{}, box, time, cfg.fgColor));
		unsigned colored = 0;
		for (int y = 0; y < gfx.h; ++y) {
			for (int x = 0; x < gfx.w; ++x) {
				auto pixel = gfx.pixels[y * gfx.w + x];
				if (pixel && pixel != cfg.fgColor) {
					++colored; assert(y >= box.y + box.h * 3 / 4 && y < box.y + box.h);
					assert(x >= box.x && x < box.x + box.w);
				}
			}
		}
		assert(time == 1500 ? colored > 0 : colored == 0);
	}
}

/** @brief Every face gets reproducible but distinct organic timing; Calm and finger gaze stay stable. */
static void shared_variety() {
	for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) {
		EyeConfig cfg{}; cfg.expression = Expression(id); cfg.asym = 1;
		EyeState state{}; EyeLife first, repeated, other, calm;
		unsigned differences = 0; float minimum = 2, maximum = 0;
		for (uint32_t time = 0; time < 20000; time += 137) {
			first.step(cfg, state, time, 33, 1, 1, 173, false);
			repeated.step(cfg, state, time, 33, 1, 1, 173, false);
			other.step(cfg, state, time, 33, 1, 1, 947, false);
			calm.step(cfg, state, time, 33, 0, 1, 947, false);
			const auto &a = first.get(), &b = repeated.get(), &c = other.get();
			assert(a.x == b.x && a.y == b.y && a.scaleX == b.scaleX && a.openL == b.openL);
			differences += a.scaleX != c.scaleX || a.x != c.x || a.y != c.y;
			assert(calm.get().scaleX == 1 && calm.get().scaleY == 1);
			assert(calm.get().x == 0 && calm.get().y == 0 && calm.get().openL == 1);
			assert(abs(a.x) <= 4 && abs(a.y) <= 3 && a.scaleX > .96f && a.scaleX < 1.04f);
			minimum = std::min(minimum, a.scaleX); maximum = std::max(maximum, a.scaleX);
		}
		assert(differences > 100 && maximum - minimum > .005f);
		state.tracking = true;
		first.step(cfg, state, 21000, 33, 1, 1, 173, false);
		assert(first.get().x == 0 && first.get().y == 0);
	}
}

/** @brief The real mask first closes, visibly detonates, scatters panels, then leaves intact configured eyes. */
static void exploding_mask() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
			eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_life_reset();
			eyes_set_eye_colors(0x0841, 0x07FF, 0xF800, 0x07E0);
			eyes_set_expression(MECHA_BREAK);
			EyeConfig cfg = eyes_config(); auto layout = eye_layout(cfg, EyeState{});
			auto render = [&](uint32_t time) {
				gfx.fillScreen(cfg.bgColor);
				assert(face_preset(MECHA_BREAK)->scene(&gfx, cfg, layout, time, 1));
				return gfx.pixels;
			};
			auto rest = render(0), closed = render(2400), impact = render(2600);
			assert(closed != rest && impact != closed);
			assert(std::count(impact.begin(), impact.end(), eye_rgb(255, 246, 210)) > 0);
			gfx.fillScreen(cfg.bgColor);
			face_defs::mecha_face::scene(&gfx, cfg, layout, 2400, 1);
			assert(gfx.pixels == closed); // The exploded panels belong to the same familiar mask.
			auto shards = render(2850);
			assert(shards != impact && shards != rest && render(2850) == shards);
			assert(render(3100) != shards);
			assert(render(4000) == rest && render(6000) == rest);
		}
	}
}

/** @brief Full cycles on every style erase cleanly and never modify user settings or the palette. */
static void redraw() {
	unsigned writes = test_flash_writes; auto profile = settings_get();
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
			for (Expression face : {SLEEPY, CURIOUS, SHY, SUSPICIOUS, LOVE, GLITCH, MECHA_BREAK, SAMURAI}) {
				eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_life_reset();
				eyes_set_eye_colors(0x0841, 0x07FF, 0xF800, 0x07E0);
				eyes_set_expression(face); eyes_state() = EyeState{};
				for (uint32_t time = 0; time < 6300; time += 137) {
					eyes_state().lastMs = time; eyes_draw(); auto dirty = gfx.pixels;
					eyes_state().dirtyInit = false; eyes_draw();
					assert(gfx.pixels == dirty);
					assert(eyes_config().fgColor == 0x07FF && eyes_config().bgColor == 0x0841);
				}
				eyes_set_expression(NEUTRAL); eyes_draw(); auto dirty = gfx.pixels;
				eyes_state().dirtyInit = false; eyes_draw(); assert(gfx.pixels == dirty);
			}
		}
	}
	assert(test_flash_writes == writes && settings_get().mood == profile.mood);
}

int main() {
	assert(settings_init());
	gestures(); glitch_recovery(); blush(); shared_variety(); exploding_mask(); redraw(); cinematic_face_checks();
	std::cout << "Emotions, mask explosion, intact recovery, blush and seven-style clean redraw passed\n";
}

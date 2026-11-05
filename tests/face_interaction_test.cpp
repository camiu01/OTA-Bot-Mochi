/**
 * @file face_interaction_test.cpp
 * @brief Random whole-cycle holds, visible handoff timing and live finger gaze.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include "../OTA-bot-Mochi/src/eyes/face_presets.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <iostream>

/** @brief All faces randomize whole loops; a late renderer, missing frame or rollover cannot cut them. */
static void schedules() {
	for (uint8_t id = 1; id < EXPRESSION_COUNT; ++id) {
		if (id == DIZZY) { continue; }
		Expression face = Expression(id);
		uint32_t cycle = FacePolicy::cycle_duration(face);
		if (face_preset(face)->normalCycle) { assert(cycle == 6000); }
		uint32_t first = FacePolicy::automatic_duration(face, 0);
		assert(first >= 6000 && first % cycle == 0);
		assert(FacePolicy::automatic_duration(face, EXPRESSION_COUNT - 2) == first + cycle);
		assert(!FacePolicy::cycle_complete(face, cycle - 1));
		assert(FacePolicy::cycle_complete(face, cycle + 33));
		assert(!FacePolicy::cycle_complete(face, cycle + 67));
		for (uint32_t start : {12000u, UINT32_MAX - 1000}) {
			FacePolicy p; p.reset(NEUTRAL, start - 12000);
			uint32_t choice = id < DIZZY ? id - 1 : id - 2;
			assert(p.update(start, true, false, choice, true, FACE_RETURN, 0) == face);
			uint32_t hold = FacePolicy::automatic_duration(face, choice);
			// Policy time is up, but the transition delayed the actual visible entry.
			assert(p.update(start + hold, true, false, choice, false, FACE_RETURN, hold - 150) == face);
			assert(p.update(start + hold + 150, true, false, choice, false, FACE_RETURN, 0) == face);
			assert(p.update(start + hold + cycle, true, false, choice, false, FACE_RETURN, hold + 100) == face);
			assert(p.update(start + hold + cycle + 100, true, false, choice, true, FACE_RETURN, hold) == NEUTRAL);
			assert(p.update(start + hold + cycle + 101, false, false, choice, false, FACE_RETURN, 0) == NEUTRAL);
		}
	}
	FacePolicy p; p.reset(NEUTRAL, 0, 9000);
	assert(p.update(12000, true, false, 0) == NEUTRAL);
	assert(p.update(21000, true, false, 0) == HAPPY);
}

/** @brief The first idle gesture varies by entry, not just its later repeat interval. */
static void idle_waits() {
	uint32_t first[2] = {};
	for (uint8_t i = 0; i < 2; ++i) {
		EyeLife life; EyeConfig cfg{}; EyeState state{};
		cfg.style = MOCHI; cfg.expression = NEUTRAL;
		for (uint32_t now = 0; now < 12000; now += 33) {
			life.step(cfg, state, now, 33, 2, 1, i ? 7000 : 0);
			if (life.get().busy) { first[i] = now; break; }
		}
	}
	assert(first[0] >= 3000 && first[0] <= 3033);
	assert(first[1] >= 10000 && first[1] <= 10033);
}

/** @brief Adversarial repeated randomness cannot align idle gestures with every scene boundary forever. */
static void idle_collision() {
	FacePolicy p; p.reset(NEUTRAL, uint32_t(-12000));
	EyeConfig cfg{}; cfg.style = MOCHI; cfg.expression = MECHA;
	EyeState state{}; EyeLife life;
	assert(p.update(0, true, false, 126, true, FACE_CONTINUOUS) == MECHA);
	bool advanced = false;
	for (uint32_t now = 0; now <= 18000; now += 50) {
		Expression face = p.update(now, true, false, 0, true, FACE_CONTINUOUS,
			life.get().busy ? 0 : now);
		if (face != MECHA) { advanced = true; break; }
		life.step(cfg, state, now, 50, 2, 1, 2000, !p.waiting(now));
	}
	assert(advanced);
}

/** @brief A held finger owns damped gaze, not the profile or expression; release resumes idle safely. */
static void following(int w, int h) {
	Arduino_GFX gfx(w, h);
	auto saved = settings_get();
	auto p = saved; p.random_faces = p.imu_faces = p.shake = false; p.dashboard = false;
	assert(settings_update(p));
	for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
		test_now = UINT32_MAX - 10000;
		eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_set_expression(NEUTRAL);
		for (const auto &point : {std::pair<int16_t, int16_t>{-32768, -32768}, {32767, 32767}}) {
			for (unsigned i = 0; i < 25; ++i) {
				test_now += 33;
				eyes_follow(point.first, point.second, true, test_now); eyes_update(test_now);
				assert(eyes_state().tracking && eyes_config().expression == NEUTRAL);
				assert(abs(eyes_state().saccadeX) <= 12 && abs(eyes_state().saccadeY) <= 8);
			}
			const auto pose = eyes_life_frame();
			assert(point.first < 0 ? pose.x <= -10 && pose.y <= -6 : pose.x >= 9 && pose.y >= 5);
			auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw();
			assert(dirty == gfx.pixels);
		}
		eyes_follow(0, 0, false, test_now);
		assert(!eyes_state().tracking);
		test_now += 600; eyes_update(test_now);
		assert(eyes_state().glanceUntilMs == 0);
		eyes_state().dizzyUntilMs = test_now + 2000;
		eyes_follow(0, 0, true, test_now);
		assert(!eyes_state().tracking);
	}
	assert(settings_get().mood == p.mood && settings_get().style == p.style);
	assert(settings_update(saved));
}

/** @brief Real 33-ms renders chain only at complete displayed loops, including delayed entry and rollover. */
static void rendered(int w, int h) {
	Arduino_GFX gfx(w, h);
	test_now = UINT32_MAX - 50000;
	eyes_init(&gfx);
	auto p = settings_get(); p.mood = NEUTRAL; p.random_faces = true;
	p.imu_faces = p.shake = false; p.face_flow = FACE_CONTINUOUS;
	p.animation = 2; p.dashboard = false; assert(settings_update(p));
	Expression previous = NEUTRAL;
	unsigned changes = 0;
	uint32_t start = test_now;
	for (uint32_t t = 0; t < 150000; t += 33) {
		test_now = start + t;
		bool touching = t >= 30000 && t < 45000;
		eyes_follow(int16_t(w / 2), int16_t(h / 2), touching, test_now);
		uint32_t entry = eyes_state().effectStartMs;
		Expression visible = eyes_state().effectExpression;
		bool busy = eyes_life_frame().busy;
		eyes_update(test_now);
		Expression next = eyes_config().expression;
		if (touching) { assert(next == previous); }
		if (previous != NEUTRAL && next != previous) {
			assert(visible == previous && !busy);
			assert(FacePolicy::cycle_complete(previous, test_now - entry));
			++changes;
		}
		previous = next;
	}
	assert(changes >= 3);
}

/** @brief Explicit affection relaxes lids; pickup feedback respects opt-ins and interaction priority. */
static void affection_and_wake(int w, int h) {
	Arduino_GFX gfx(w, h);
	auto p = settings_get(); p.random_faces = false; p.imu_faces = true;
	p.mood = NEUTRAL; p.animation = 0; assert(settings_update(p));
	for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
		test_now = UINT32_MAX - 2000;
		eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_set_expression(NEUTRAL);
		eyes_state().nextBlinkMs = test_now + 100000;
		for (unsigned i = 0; i < 45; ++i) {
			test_now += 33; eyes_follow(w / 2, h / 2, true, test_now);
			if (i % 10 == 0) { eyes_faces_pet(test_now, w / 2, h / 2); }
			eyes_update(test_now);
		}
		assert(eyes_life_frame().openL < 0.8f && eyes_life_frame().openR < 0.8f);
		eyes_wake(test_now); assert(!eyes_state().wakeUntilMs);
		eyes_follow(0, 0, false, test_now);
		for (unsigned i = 0; i < 150; ++i) { test_now += 33; eyes_update(test_now); }
		assert(eyes_life_frame().openL > 0.99f);
		eyes_wake(test_now); assert(eyes_state().wakeUntilMs);
		test_now += 33; eyes_update(test_now);
		assert(eyes_life_frame().openL < 0.7f);
		auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw(); assert(dirty == gfx.pixels);
		eyes_follow(w / 2, h / 2, true, test_now); assert(!eyes_state().wakeUntilMs);
		eyes_follow(0, 0, false, test_now);
		for (unsigned i = 0; i < 60; ++i) { test_now += 33; eyes_update(test_now); }
		assert(eyes_life_frame().openL > 0.99f);
		eyes_state().dizzyUntilMs = test_now + 2000;
		eyes_wake(test_now); assert(!eyes_state().wakeUntilMs);
		eyes_state().dizzyUntilMs = 0;
		assert(settings_get().mood == p.mood && settings_get().style == p.style);
	}
	p.imu_faces = false; assert(settings_update(p));
	eyes_wake(test_now); assert(!eyes_state().wakeUntilMs);
}

int main() {
	assert(settings_init());
	schedules(); idle_waits(); idle_collision();
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		following(panel.first, panel.second);
		rendered(panel.first, panel.second);
		affection_and_wake(panel.first, panel.second);
	}
	std::cout << "Random complete loops, delayed visible handoff, rollover and live finger gaze passed\n";
}

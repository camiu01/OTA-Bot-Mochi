/**
 * @file eye_play_checks.h
 * @brief Rare-gag deadlines, contact priorities and dirty/full renderer equivalence.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
/** @brief Every rare gag waits at least 45s, finishes in 1100ms and respects handoff reservation/rollover. */
static void rare_play_checks() {
	for (uint32_t start : {0u, UINT32_MAX - 400}) {
		for (uint32_t choice = 0; choice < 3; ++choice) {
			EyePlay play; EyeState st{};
			assert(!play.step(st, start, 1, true, true, choice).busy);
			assert(!play.step(st, start + 44999, 1, true, true, choice).busy);
			uint32_t due = start + 45000 + choice;
			assert(!play.step(st, due, 1, true, false, choice).busy);
			assert(play.step(st, due + 33, 1, true, true, choice).busy);
			auto pose = play.step(st, due + 583, 1, true, true, choice);
			assert(pose.busy && pose.openL >= 0 && pose.openR >= 0);
			if (choice == 0) { assert(abs(pose.x) >= 7 && pose.openL < 0.4f); }
			if (choice == 1) { assert(pose.rightY >= 23 && pose.leftY <= -2); }
			if (choice == 2) { assert(pose.fly && pose.flyY < -40); }
			assert(!play.step(st, due + 1133, 1, true, true, choice).busy);
			assert(!play.step(st, due + 1200, 1, true, true, choice).busy);
			st.tracking = true; assert(play.step(st, due + 1300, 1, true, true, choice).busy);
			assert(!play.step(st, due + 1400, 1, false, true, choice).busy);
			assert(!play.step(st, due + 1500, 1, true, true, choice).busy); // No queued contact.
		}
	}
}
/** @brief Center tickles, edge dodges and repeated contacts sulk, without retriggering a held finger. */
static void touch_play_checks() {
	EyePlay play; EyeState st{}; play.step(st, 0, 1, true, true, 0);
	for (uint32_t contact : {100u, 800u, 1500u}) {
		st.tracking = true; st.saccadeX = contact == 800 ? 12 : 0;
		play.step(st, contact, 1, true, true, 0);
		auto peak = play.step(st, contact + (contact == 1500 ? 450 : 210), 1, true, true, 0);
		if (contact == 100) { assert(peak.openL < 0.81f && peak.openR < 0.71f); }
		if (contact == 800) { assert(peak.x <= -7 && peak.openL == 1); }
		if (contact == 1500) { assert(peak.openL < 0.56f && peak.openR < 0.56f); }
		st.tracking = false; play.step(st, contact + 500, 1, true, true, 0);
	}
	st.tracking = true; play.step(st, 4000, 1, true, true, 0);
	assert(!play.step(st, 5000, 1, true, true, 0).busy);
	assert(!play.step(st, 6000, 1, true, true, 0).busy);
	st.tracking = false; play.step(st, 6100, 1, false, true, 0);
	assert(!play.step(st, 70000, 1, false, true, 0).busy);
}
/** @brief Actual released taps retain the sulk across ordinary handoffs; recognized petting supersedes it. */
static void released_tap_play_checks(int w, int h) {
	auto saved = settings_get(); auto filter = face_filter(); auto p = saved;
	p.random_faces = p.imu_faces = p.shake = p.dashboard = false; p.animation = 2; p.mood = NEUTRAL;
	assert(settings_update(p)); FaceFilter ordinary;
	for (uint16_t id = 0; id < EXPRESSION_COUNT; ++id) {
		if (id != NEUTRAL && id != HAPPY) { ordinary.set(Expression(id), false); }
	}
	assert(settings_faces_update(ordinary));
	Arduino_GFX gfx(w, h); test_now = 0; eyes_init(&gfx); settings_apply();
	bool previous = false, sulked = false; unsigned writes = test_flash_writes;
	for (uint32_t t = 0; t < 3500; t += 33) {
		bool contact = (t >= 1000 && t < 1100) || (t >= 1800 && t < 1900) || (t >= 2600 && t < 2700);
		test_now = t; eyes_follow(w / 2, h / 2, contact, t);
		if (previous && !contact) { eyes_faces_tap(t, w / 2, h / 2); }
		previous = contact;
		if (t >= 3200) { eyes_faces_pet(t, w / 2, h / 2); }
		eyes_update(t);
		if (t >= 3030 && t < 3100) { sulked |= eyes_life_frame().play.openL < 0.55f; }
		if (t >= 3200) { assert(!eyes_life_frame().play.busy && !eyes_life_frame().play.fly); }
	}
	assert(sulked && settings_get().mood == NEUTRAL && test_flash_writes == writes);
	assert(settings_faces_update(filter) && settings_update(saved));
}
/** @brief Exercise production updates and repaint paths, including the fly disappearing after a touch. */
static void rendered_play_checks(int w, int h) {
	Arduino_GFX gfx(w, h);
	auto saved = settings_get(), p = saved;
	p.random_faces = p.imu_faces = p.shake = p.dashboard = false; p.animation = 2; p.mood = NEUTRAL;
	assert(settings_update(p));
	for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
		std::srand(1); test_now = 0; eyes_init(&gfx); eyes_set_style(EyeStyle(style));
		bool seen = false, seen_fly = false;
		for (uint32_t t = 0; t < 300000; t += 33) {
			test_now = t; eyes_update(t);
			const auto &play = eyes_life_frame().play;
			if (play.busy) {
				seen = true; seen_fly |= play.fly;
				auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw(); assert(dirty == gfx.pixels);
			}
			if (seen_fly && !play.fly) { break; }
		}
		assert(seen && seen_fly);
		auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw(); assert(dirty == gfx.pixels);
		eyes_follow(w / 2, h / 2, true, test_now); test_now += 33; eyes_update(test_now);
		assert(!eyes_life_frame().play.fly && settings_get().mood == NEUTRAL && eyes_config().expression == NEUTRAL);
	}
	assert(settings_update(saved));
}

/**
 * @file petting_gesture_regressions.h
 * @brief Slow, sparse, noisy and rollover petting samples remain separate from navigation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief One-pixel strokes work without reversal and respect the repeat cooldown across rollover. */
inline void gentle_pet_tests() {
	for (uint32_t start : {0u, UINT32_MAX - 150}) {
		TouchActions actions;
		actions.update(true, 120, 100, start);
		uint32_t now = start;
		for (int16_t y = 101; y <= 132; ++y) { actions.update(true, 120, y, now += 8); }
		assert(actions.take_pet());
		for (int16_t y = 131; y >= 100; --y) { actions.update(true, 124, y, now += 8); }
		assert(!actions.take_pet());
		for (int16_t y = 101; y <= 132; ++y) { actions.update(true, 124, y, now += 8); }
		int16_t x = 0, y = 0;
		assert(actions.take_pet(x, y) && x == 124 && y == 132);
		assert(actions.vertical && !actions.tap && !actions.hold && !actions.scroll);
		actions.update(false, 0, 0, now + 10);
		assert(!actions.tap && !actions.take_double() && !actions.take_pet());
	}
	TouchActions sparse;
	sparse.update(true, 120, 100, 0);
	sparse.update(true, 122, 128, 100);
	sparse.update(true, 124, 100, 220);
	assert(sparse.take_pet());
	// Continuing contact without motion cannot renew affection.
	sparse.update(true, 124, 100, 5000);
	assert(!sparse.take_pet() && !sparse.hold);
}

/** @brief Rising/falling diagonal strokes tolerate drift without also navigating to another page. */
inline void single_direction_pet_tests() {
	for (int16_t direction : {-1, 1}) {
		for (int16_t lateral : {-63, -55, 0, 55, 63}) {
			TouchActions actions;
			SwipeTracker navigation;
			actions.update(true, 120, 100, 0);
			navigation.update(true, 120, 100, 0);
			for (int16_t step = 1; step <= 36; ++step) {
				actions.update(true, int16_t(120 + lateral * step / 36),
					int16_t(100 + direction * step), uint32_t(step) * 10);
				int8_t page = navigation.update(!actions.vertical, actions.x, actions.y, uint32_t(step) * 10);
				assert(page == 0);
			}
			assert(actions.take_pet() && actions.vertical && !actions.horizontal);
			actions.update(false, 0, 0, 400);
			assert(!actions.tap && !actions.take_double() && !actions.hold);
		}
	}
	TouchActions wide;
	wide.update(true, 120, 100, 0);
	wide.update(true, 184, 140, 200);
	assert(!wide.take_pet());
}

/** @brief Short up/down rubbing qualifies even when neither leg reaches the single-pass threshold. */
inline void repeated_pet_tests() {
	for (int16_t direction : {-1, 1}) {
		TouchActions actions;
		actions.update(true, 120, 100, 0);
		uint32_t now = 0;
		for (int16_t step = 1; step <= 24; ++step) {
			actions.update(true, 124, int16_t(100 + direction * step), now += 10);
		}
		assert(!actions.take_pet());
		for (int16_t step = 23; step >= 0; --step) {
			actions.update(true, 124, int16_t(100 + direction * step), now += 10);
		}
		assert(actions.take_pet() && actions.vertical);
		actions.update(false, 0, 0, now + 10);
		assert(!actions.tap && !actions.hold && !actions.take_double());
		// A new contact must qualify its own vertical displacement.
		actions.update(true, 120, 100, now + 30);
		for (int16_t step = 1; step <= 8; ++step) {
			actions.update(true, 120, int16_t(100 + (step % 2 ? 16 : 0)), now += 100);
			assert(!actions.take_pet());
		}
	}
}

/** @brief Pet sampling must not shrink the existing 18-pixel tap box or consume minor tap jitter. */
inline void drifting_tap_tests() {
	for (int16_t direction : {-1, 1}) {
		TouchActions actions;
		actions.update(true, 100, 100, 0);
		actions.update(true, int16_t(100 + direction * 16), int16_t(100 + direction * 16), 500);
		assert(!actions.moved && !actions.take_pet());
		actions.update(false, 0, 0, 700);
		assert(actions.tap);
	}
	TouchActions jitter;
	jitter.update(true, 100, 100, 0);
	for (int16_t step = 1; step <= 6; ++step) {
		jitter.update(true, 100, int16_t(100 + (step % 2 ? 16 : 0)), uint32_t(step) * 100);
		assert(!jitter.take_pet() && !jitter.moved);
	}
	jitter.update(false, 0, 0, 700);
	assert(jitter.tap);
}

/** @brief Finger jitter, isolated position jumps and a claimed horizontal swipe never pet. */
inline void rejected_pet_tests() {
	TouchActions jitter;
	jitter.update(true, 120, 100, 0);
	for (uint32_t now = 10; now <= 1500; now += 10) {
		jitter.update(true, 120, int16_t(100 + (now % 30 == 0 ? 3 : -3)), now);
		assert(!jitter.take_pet());
	}
	assert(jitter.hold);
	TouchActions jump;
	jump.update(true, 120, 100, 0);
	jump.update(true, 120, 230, 200);
	jump.update(true, 120, 100, 220);
	assert(!jump.take_pet());
	TouchActions swipe;
	swipe.update(true, 120, 100, 0);
	swipe.update(true, 175, 102, 100);
	assert(swipe.horizontal);
	for (int16_t y : {130, 100, 130, 100}) { swipe.update(true, 120, y, 400); }
	assert(!swipe.take_pet());
}

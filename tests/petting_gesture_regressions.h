/**
 * @file petting_gesture_regressions.h
 * @brief Slow, sparse, noisy and rollover petting samples remain separate from navigation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief One-pixel samples accumulate; one deliberate reversal is enough, including across rollover. */
inline void gentle_pet_tests() {
	for (uint32_t start : {0u, UINT32_MAX - 150}) {
		TouchActions actions;
		actions.update(true, 120, 100, start);
		uint32_t now = start;
		for (int16_t y = 101; y <= 132; ++y) { actions.update(true, 120, y, now += 8); }
		assert(!actions.take_pet());
		for (int16_t y = 131; y >= 100; --y) { actions.update(true, 124, y, now += 8); }
		int16_t x = 0, y = 0;
		assert(actions.take_pet(x, y) && x == 124 && y == 100);
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

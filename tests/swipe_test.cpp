/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/touch/swipe.h"
#include "../OTA-bot-Mochi/src/touch/touch_actions.h"
#include <assert.h>
#include <stdint.h>

/** @brief Stroke down-up-down with small steps while keeping X steady. */
static void stroke(TouchActions &actions, int16_t x, uint32_t &now) {
	int16_t legs[] = {130, 100, 130};
	for (uint8_t leg = 0; leg < 3; ++leg) {
		int16_t target = legs[leg];
		while (actions.y != target) {
			int16_t next = actions.y + (target > actions.y ? 5 : -5);
			now += 20;
			actions.update(true, x, next, now);
		}
	}
}

/** @brief Two nearby quick taps double; distant, slow, or cancelled taps stay single. */
static void double_tap_tests() {
	TouchActions actions;
	actions.update(true, 120, 100, 0);
	actions.update(false, 0, 0, 100);
	assert(actions.tap && !actions.take_double());
	actions.update(true, 124, 104, 200);
	actions.update(false, 0, 0, 260);
	assert(actions.tap && actions.take_double());
	assert(!actions.take_double());

	// A third tap starts a new pair instead of chaining doubles.
	actions.update(true, 120, 100, 400);
	actions.update(false, 0, 0, 460);
	assert(actions.tap && !actions.take_double());

	TouchActions slow;
	slow.update(true, 120, 100, 0);
	slow.update(false, 0, 0, 100);
	slow.update(true, 120, 100, 600);
	slow.update(false, 0, 0, 700);
	assert(slow.tap && !slow.take_double());

	TouchActions wide;
	wide.update(true, 40, 100, 0);
	wide.update(false, 0, 0, 100);
	wide.update(true, 90, 100, 200);
	wide.update(false, 0, 0, 300);
	assert(wide.tap && !wide.take_double());

	TouchActions cleared;
	cleared.update(true, 120, 100, 0);
	cleared.update(false, 0, 0, 100);
	cleared.update(true, 120, 100, 200);
	cleared.update(false, 0, 0, 260);
	assert(cleared.double_tap);
	cleared.cancel();
	assert(!cleared.take_double() && !cleared.tap);
}

/** @brief Vertical rubbing pets; taps, single strokes, and swipes do not. */
static void pet_tests() {
	TouchActions tap;
	tap.update(true, 120, 100, 0);
	tap.update(false, 0, 0, 100);
	assert(!tap.take_pet());
	assert(tap.tap);

	TouchActions single;
	uint32_t now = 0;
	single.update(true, 120, 100, now);
	for (int16_t next = 105; next <= 160; next += 5) {
		now += 20;
		single.update(true, 121, next, now);
	}
	assert(!single.take_pet());
	single.update(false, 0, 0, now + 20);
	assert(!single.take_pet());

	TouchActions swipe;
	now = 0;
	swipe.update(true, 150, 100, now);
	for (int16_t next = 140; next >= 80; next -= 10) {
		now += 20;
		swipe.update(true, next, 102, now);
	}
	assert(!swipe.take_pet());

	TouchActions actions;
	now = 0;
	actions.update(true, 120, 100, now);
	stroke(actions, 120, now);
	assert(actions.take_pet());
	assert(!actions.take_pet());
	assert(!actions.tap && !actions.hold);

	actions.update(true, 120, actions.y, now + 100);
	stroke(actions, 120, now);
	assert(!actions.take_pet());

	now += 1600;
	stroke(actions, 120, now);
	assert(actions.take_pet());
	actions.update(false, 0, 0, now + 20);
	assert(actions.take_pet() == false);

	TouchActions wide;
	now = 0;
	wide.update(true, 100, 100, now);
	for (uint8_t leg = 0; leg < 3; ++leg) {
		for (uint8_t step = 0; step < 6; ++step) {
			now += 20;
			int16_t ny = leg % 2 ? 100 + step * 5 : 130 - step * 5;
			wide.update(true, int16_t(100 + step * 10), ny, now);
		}
	}
	assert(!wide.take_pet());

	TouchActions cancelled;
	now = 0;
	cancelled.update(true, 120, 100, now);
	stroke(cancelled, 120, now);
	assert(cancelled.take_pet());
	cancelled.cancel();
	assert(!cancelled.take_pet());
}

int main() {
	SwipeTracker swipe;
	assert(swipe.update(true, 150, 100, 0) == 0);
	assert(swipe.update(true, 130, 100, 20) == 0);
	assert(swipe.update(true, 110, 100, 40) == 1);
	assert(swipe.update(true, 70, 100, 60) == 0);
	assert(swipe.update(false, 0, 0, 80) == 0);
	assert(swipe.update(true, 50, 100, 100) == 0);
	assert(swipe.update(true, 100, 110, 120) == -1);
	assert(swipe.update(false, 0, 0, 140) == 0);
	assert(swipe.update(true, 100, 100, 160) == 0);
	assert(swipe.update(true, 105, 160, 180) == 0);
	assert(swipe.update(false, 0, 0, 200) == 0);
	assert(swipe.update(true, 150, 100, 220) == 0);
	assert(swipe.update(true, 70, 100, 1221) == 0);
	assert(swipe.update(false, 0, 0, 1240) == 0);
	assert(swipe.update(true, 150, 100, UINT32_MAX - 10) == 0);
	assert(swipe.update(true, 100, 100, 20) == 1);
	double_tap_tests();
	pet_tests();
	return 0;
}

/**
 * @file touch_actions.h
 * @brief Separate taps and stationary holds from swipes, independent of hardware.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct TouchActions {
	bool active = false;
	bool moved = false;
	bool held = false;
	bool tap = false;
	bool hold = false;
	int16_t x = 0, y = 0;
	int16_t start_x = 0, start_y = 0;
	uint32_t started = 0;

	/** @brief Cancel input after an invalid hardware sample. */
	void cancel() { active = false; tap = false; hold = false; }

	/**
	 * @brief Recognize short taps and one stationary hold per contact.
	 * @param touching Finger contact.
	 * @param next_x X coordinate.
	 * @param next_y Y coordinate.
	 * @param now Timestamp.
	 */
	void update(bool touching, int16_t next_x, int16_t next_y, uint32_t now) {
		if (!touching) {
			if (active && !moved && !held && uint32_t(now - started) <= 600) { tap = true; }
			active = false;
			return;
		}
		x = next_x; y = next_y;
		if (!active) {
			active = true; moved = false; held = false;
			start_x = x; start_y = y; started = now;
		}
		int32_t dx = int32_t(x) - start_x, dy = int32_t(y) - start_y;
		if (dx < -12 || dx > 12 || dy < -12 || dy > 12) { moved = true; }
		if (!moved && !held && uint32_t(now - started) >= 1200) {
			held = true; hold = true;
		}
	}
};

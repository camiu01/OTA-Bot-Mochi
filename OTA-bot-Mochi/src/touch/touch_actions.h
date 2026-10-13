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
	bool vertical = false;
	bool horizontal = false;
	int8_t scroll = 0;
	int16_t x = 0, y = 0;
	int16_t start_x = 0, start_y = 0;
	uint32_t started = 0;

	/** @brief Cancel input after an invalid hardware sample. */
	void cancel() { active = false; tap = false; hold = false; scroll = 0; vertical = horizontal = false; }

	/**
	 * @brief Recognize short taps and one stationary hold per contact.
	 * @param touching Finger contact.
	 * @param next_x X coordinate.
	 * @param next_y Y coordinate.
	 * @param now Timestamp.
	 */
	void update(bool touching, int16_t next_x, int16_t next_y, uint32_t now) {
		if (!touching) {
			if (active && !moved && !held && uint32_t(now - started) <= 800) { tap = true; }
			active = false;
			return;
		}
		x = next_x; y = next_y;
		if (!active) {
			active = true; moved = false; held = false;
			vertical = horizontal = false;
			start_x = x; start_y = y; started = now;
		}
		int32_t dx = int32_t(x) - start_x, dy = int32_t(y) - start_y;
		if (dx < -18 || dx > 18 || dy < -18 || dy > 18) { moved = true; }
		int32_t ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
		if (!held && !vertical && !horizontal) {
			if (ay >= 28 && ay >= ax * 2) { vertical = true; scroll = dy < 0 ? 1 : -1; }
			else if (ax >= 40 && ax >= ay * 2) { horizontal = true; }
		}
		if (!moved && !held && uint32_t(now - started) >= 1200) {
			held = true; hold = true;
		}
	}
};

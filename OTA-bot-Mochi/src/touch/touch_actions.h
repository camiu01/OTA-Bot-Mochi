/**
 * @file touch_actions.h
 * @brief Separate taps, stationary holds, and petting strokes from swipes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct TouchActions {
	bool active = false;
	bool moved = false;
	bool held = false;
	bool tap = false;
	bool double_tap = false;
	bool hold = false;
	bool pet = false;
	bool vertical = false;
	bool horizontal = false;
	int8_t scroll = 0;
	int16_t x = 0, y = 0;
	int16_t start_x = 0, start_y = 0;
	int16_t last_y = 0, anchor_y = 0;
	int16_t last_tap_x = 0, last_tap_y = 0;
	int8_t last_dir = 0;
	uint8_t flips = 0;
	uint16_t travel = 0;
	uint32_t started = 0;
	uint32_t last_pet = 0;
	uint32_t last_tap_ms = 0;
	bool tap_armed = false;
	bool pet_once = false;

	/** @brief Cancel input after an invalid hardware sample. */
	void cancel() {
		active = false; tap = false; double_tap = false; hold = false; pet = false;
		scroll = 0; vertical = horizontal = false;
		flips = 0; travel = 0; last_dir = 0;
	}

	/**
	 * @brief Consume a recognized double tap.
	 * @return True once per double tap.
	 */
	bool take_double() {
		bool doubled = double_tap;
		double_tap = false;
		return doubled;
	}

	/**
	 * @brief Consume a recognized petting stroke.
	 * @return True once per stroke.
	 */
	bool take_pet() {
		bool stroked = pet;
		pet = false;
		return stroked;
	}

	/**
	 * @brief Track one vertical sample for an up-down petting stroke.
	 * @param now Timestamp.
	 */
	void pet_sample(uint32_t now) {
		int32_t dy = int32_t(y) - last_y;
		last_y = y;
		if (dy < -20 || dy > 20 || (dy >= -2 && dy <= 2)) { return; }
		int8_t dir = dy > 0 ? 1 : -1;
		uint16_t step = uint16_t(dy < 0 ? -dy : dy);
		travel = uint16_t(travel + step);
		if (last_dir == 0) {
			last_dir = dir;
			return;
		}
		if (dir != last_dir) {
			int32_t seg = int32_t(y) - anchor_y;
			if (seg < 0) { seg = -seg; }
			if (seg >= 14) {
				flips++;
				anchor_y = y;
				last_dir = dir;
			}
		}
		if (flips < 2 || travel < 60) { return; }
		int32_t ax = int32_t(x) - start_x;
		if (ax < 0) { ax = -ax; }
		if (ax >= 30) { return; }
		if (uint32_t(now - started) < 200) { return; }
		if (pet_once && uint32_t(now - last_pet) < 1500) { return; }
		pet = true;
		last_pet = now;
		pet_once = true;
		flips = 0;
		travel = 0;
		last_dir = 0;
		anchor_y = y;
	}

	/**
	 * @brief Recognize short taps, double taps, one stationary hold, and petting strokes per contact.
	 * @param touching Finger contact.
	 * @param next_x X coordinate.
	 * @param next_y Y coordinate.
	 * @param now Timestamp.
	 */
	void update(bool touching, int16_t next_x, int16_t next_y, uint32_t now) {
		if (!touching) {
			if (active && !moved && !held && uint32_t(now - started) <= 800) {
				tap = true;
				int32_t dx = int32_t(x) - last_tap_x, dy = int32_t(y) - last_tap_y;
				if (tap_armed && uint32_t(now - last_tap_ms) <= 350 &&
					dx >= -30 && dx <= 30 && dy >= -30 && dy <= 30) {
					double_tap = true;
					tap_armed = false;
				} else {
					tap_armed = true;
				}
				last_tap_ms = now;
				last_tap_x = x;
				last_tap_y = y;
			}
			active = false;
			return;
		}
		x = next_x; y = next_y;
		if (!active) {
			active = true; moved = false; held = false;
			vertical = horizontal = false;
			start_x = x; start_y = y; started = now;
			last_y = y; anchor_y = y; last_dir = 0;
			flips = 0; travel = 0;
			return;
		}
		pet_sample(now);
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

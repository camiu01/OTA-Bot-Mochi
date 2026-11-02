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
	uint16_t travel = 0;
	uint32_t started = 0;
	uint32_t last_pet = 0;
	uint32_t last_tap_ms = 0;
	bool tap_armed = false;
	bool pet_once = false;
	bool pet_displaced = false;

	/** @brief Cancel input after an invalid hardware sample. */
	void cancel() {
		active = false; tap = false; double_tap = false; hold = false; pet = false;
		scroll = 0; vertical = horizontal = false;
		travel = 0; last_dir = 0;
		tap_armed = false; pet_once = false; pet_displaced = false;
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
	 * @brief Consume a recognized petting stroke with the current contact position.
	 * @param px Output stroke X.
	 * @param py Output stroke Y.
	 * @return True once per stroke.
	 */
	bool take_pet(int16_t &px, int16_t &py) {
		px = x; py = y;
		return take_pet();
	}

	/**
	 * @brief Track a vertical stroke in either direction, allowing moderate sideways drift.
	 * @param now Timestamp.
	 */
	void pet_sample(uint32_t now) {
		int32_t ax = int32_t(x) - start_x;
		int32_t jump = int32_t(y) - last_y;
		if (held || horizontal || ax <= -64 || ax >= 64 || jump < -80 || jump > 80) {
			anchor_y = last_y = y; last_dir = 0; travel = 0;
			pet_displaced = false;
			return;
		}
		if (last_dir == 0) {
			int32_t dy = int32_t(y) - anchor_y;
			if (dy > -12 && dy < 12) { return; }
			last_dir = dy > 0 ? 1 : -1;
			travel = uint16_t(dy * last_dir); last_y = y;
		} else {
			int32_t advance = (int32_t(y) - last_y) * last_dir;
			if (advance > 0 || advance <= -12) {
				if (advance < 0) { last_dir = -last_dir; advance = -advance; }
				uint32_t total = uint32_t(travel) + uint32_t(advance);
				travel = uint16_t(total > UINT16_MAX ? UINT16_MAX : total);
				last_y = y;
			}
		}
		int32_t extent = int32_t(y) - start_y;
		if (extent <= -20 || extent >= 20) { pet_displaced = true; }
		if (!pet_displaced || travel < 32 ||
			uint32_t(now - started) < 120) { return; }
		if (pet_once && uint32_t(now - last_pet) < 400) { return; }
		pet = true; moved = true; vertical = true; scroll = 0; tap_armed = false;
		last_pet = now;
		pet_once = true;
		travel = 0;
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
			travel = 0;
			pet_displaced = false;
			return;
		}
		pet_sample(now);
		int32_t dx = int32_t(x) - start_x, dy = int32_t(y) - start_y;
		if (dx < -18 || dx > 18 || dy < -18 || dy > 18) { moved = true; }
		if (moved) { tap_armed = false; }
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

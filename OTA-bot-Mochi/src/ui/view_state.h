/**
 * @file view_state.h
 * @brief Session-only route presentation and bounded non-blocking desk timer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

class DeskTimer {
	uint32_t last = 0;
public:
	uint8_t mode = 2;
	uint32_t remaining = 1500000;
	bool running = false, finished = false, resting = false;
	/** @brief Reset without starting. @param now Timestamp. */
	void reset(uint32_t now) {
		remaining = mode == 0 ? 300000 : (mode == 1 ? 900000 : 1500000);
		last = now; running = finished = resting = false;
	}
	/** @brief Advance across rollover, pause, and one Pomodoro work/rest pair. @param now Timestamp. */
	void tick(uint32_t now) {
		uint32_t delta = now - last; last = now;
		if (!running) { return; }
		if (delta < remaining) { remaining -= delta; return; }
		delta -= remaining;
		if (mode == 2 && !resting) {
			resting = true; remaining = 300000;
			if (delta < remaining) { remaining -= delta; return; }
		}
		remaining = 0; running = false; finished = true;
	}
	/** @brief Start, pause, resume, or restart completed timer. @param now Timestamp. */
	void toggle(uint32_t now) {
		tick(now);
		if (finished) { reset(now); }
		running = !running; last = now;
	}
	/** @brief Select 5min, 15min, or Pomodoro; reset paused. @param now Timestamp. */
	void cycle(uint32_t now) { mode = (mode + 1) % 3; reset(now); }
};

/** @brief Resolve temporary override or default. @param preference Saved/RAM default. @return Large layout. */
bool view_route_large(bool preference);
/** @brief Set a session-only layout. @param mode -1 default, 0 details, 1 large. */
void view_route_temporary(int8_t mode);
/** @brief Restore boot-like session state; never touch NVS. */
void view_reset();
/** @brief Track presentation changes for immediate redraw. @return Revision. */
uint32_t view_revision();
/** @brief Access the single session timer. @return Timer. */
DeskTimer &view_timer();

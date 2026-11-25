/**
 * @file view_state.h
 * @brief Session-only route presentation and bounded non-blocking desk timer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Countdowns (5, 15, Pomodoro 25/5, tea 3, pasta 10), a repeating 30 min break reminder, a custom countdown and a stopwatch. */
class DeskTimer {
	uint32_t last = 0;
public:
	static constexpr uint8_t BREAK = 5, CUSTOM = 6, STOPWATCH = 7, MODES = 8;
	static constexpr uint32_t LENGTH[MODES] = {300000, 900000, 1500000, 180000, 600000, 1800000, 0, 0};
	static constexpr uint32_t MINUTE = 60000, CUSTOM_MAX = 180 * MINUTE;
	uint32_t custom = 10 * MINUTE;	/**< Custom countdown length, 1..180 min, RAM-only. */
	static constexpr uint32_t STOPWATCH_MAX = 359999000;	/**< 99:59:59, then the stopwatch stops. */
	uint8_t mode = 2;
	uint32_t remaining = 1500000;	/**< Countdown left, or elapsed time in stopwatch mode (ms). */
	bool running = false, finished = false, resting = false;
	uint16_t reminders = 0;	/**< Break reminders fired; the main loop turns each into one alert. */
	/** @brief Reset without starting. @param now Timestamp. */
	void reset(uint32_t now) {
		remaining = length();
		last = now; running = finished = resting = false;
	}
	/** @brief Advance across rollover, pause, and one Pomodoro work/rest pair. @param now Timestamp. */
	void tick(uint32_t now) {
		uint32_t delta = now - last; last = now;
		if (!running) { return; }
		if (mode == STOPWATCH) {
			remaining = delta < STOPWATCH_MAX - remaining ? remaining + delta : STOPWATCH_MAX;
			if (remaining == STOPWATCH_MAX) { running = false; finished = true; }
			return;
		}
		if (delta < remaining) { remaining -= delta; return; }
		delta -= remaining;
		if (mode == BREAK) {	// Fire once and start the next interval; a long gap does not stack alerts.
			++reminders; remaining = LENGTH[BREAK] - delta % LENGTH[BREAK]; return;
		}
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
	/** @brief Current mode's countdown length. @return Milliseconds, 0 for the stopwatch. */
	uint32_t length() const { return mode == CUSTOM ? custom : LENGTH[mode]; }
	/** @brief Change the paused custom countdown by whole minutes, clamped to 1..180. @param minutes Step. @param now Timestamp. @return Changed. */
	bool adjust(int16_t minutes, uint32_t now) {
		if (mode != CUSTOM || running) { return false; }
		int32_t next = int32_t(custom) + int32_t(minutes) * int32_t(MINUTE);
		custom = uint32_t(next < int32_t(MINUTE) ? MINUTE : (next > int32_t(CUSTOM_MAX) ? CUSTOM_MAX : next));
		reset(now); return true;
	}
	/** @brief Select the next mode; reset paused. @param now Timestamp. */
	void cycle(uint32_t now) { mode = (mode + 1) % MODES; reset(now); }
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

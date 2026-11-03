/**
 * @file battery_test.cpp
 * @brief Verify per-board display endpoints, monotonicity and retained plausibility gating.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/battery.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <cassert>

/** @brief Confirm calibrated AMOLED endpoints without changing the LCD curve. @return Success. */
int main() {
#if defined(MOCHI_BOARD_AMOLED_18)
	assert(battery_percent_mv(3180) == 0);
	assert(battery_percent_mv(4088) == 100);
	assert(battery_percent_mv(3820) == 53);
	assert(battery_percent_mv(3300) == 2);
#else
	assert(battery_percent_mv(3180) == 3);
	assert(battery_percent_mv(4088) == 92);
	assert(battery_percent_mv(3820) == 50);
	assert(battery_percent_mv(3300) == 5);
#endif
	assert(battery_percent_mv(0) == 0 && battery_percent_mv(65535) == 100);
	uint8_t previous = 0;
	for (uint16_t mv = 2800; mv <= 4350; ++mv) {
		uint8_t percent = battery_percent_mv(mv);
		assert(percent >= previous && percent <= 100);
		previous = percent;
	}
	BatteryTracker tracker;
	assert(tracker.percent == BATTERY_UNAVAILABLE);
	tracker.update_mv(4088); tracker.update_mv(4088);
	assert(tracker.percent == BATTERY_UNAVAILABLE);
	tracker.update_mv(4088);
	assert(tracker.percent == battery_percent_mv(4088));
	for (int i = 0; i < 5; ++i) { tracker.update_mv(0); }
	assert(tracker.percent == BATTERY_UNAVAILABLE);
}

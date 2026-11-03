/**
 * @file battery_chronos_test.cpp
 * @brief Verify valid calibrated values and charge state reach the Chronos setter, never unknown sentinels.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/battery_chronos.h"
#include <cassert>

namespace {
uint8_t percent = BATTERY_UNAVAILABLE;
bool charging = false;
}
/** @brief Supply the current displayed estimate. @return Percent or unknown sentinel. */
uint8_t battery_percent() { return percent; }
/** @brief Supply the stable PMU charge state. @return Charging. */
bool charging_active() { return charging; }

/** @brief Cover initial unknown, changing percentages, charge edges and later read failure. @return Success. */
int main() {
	ChronosESP32 watch;
	battery_chronos_poll(watch);
	assert(watch.battery_updates == 0);
	for (uint8_t value : {uint8_t(0), uint8_t(53), uint8_t(100)}) {
		percent = value;
		battery_chronos_poll(watch);
		assert(watch.reported_battery == value && !watch.reported_charging);
	}
	charging = true;
	battery_chronos_poll(watch);
	assert(watch.reported_battery == 100 && watch.reported_charging);
	charging = false;
	battery_chronos_poll(watch);
	assert(!watch.reported_charging);
	unsigned updates = watch.battery_updates;
	percent = BATTERY_UNAVAILABLE;
	battery_chronos_poll(watch);
	assert(watch.battery_updates == updates && watch.reported_battery == 100);
}

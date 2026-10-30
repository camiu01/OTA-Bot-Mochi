/**
 * @file device_status_test.cpp
 * @brief Exercise platform capture without claiming physical ESP32 measurements.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/diagnostics/device_status.h"
#include <cassert>
#include <cstring>
#include <iostream>

/** @brief Check capability pools, uptime beyond millis rollover and observation-only Wi-Fi. @return Success. */
int main() {
	test_uptime_us = 5000000000000LL;
	WiFi.connection = WL_CONNECTED;
	auto state = device_status::snapshot();
	assert(state.measured && state.wifi && state.uptime == 5000000000ULL);
	assert(state.heap == test_internal_free && state.minimum_heap == test_internal_min);
	assert(state.psram == test_psram_free);
	WiFi.connection = 0; test_psram_free = 0;
	state = device_status::snapshot();
	assert(!state.wifi && state.psram == 0);
	assert(WiFi.station_begins == 0 && WiFi.station_disconnects == 0 && WiFi.scan_starts == 0);
	const char *labels[] = {"Other", "Power on", "Other", "Software", "Panic",
		"Watchdog", "Watchdog", "Watchdog", "Deep sleep", "Brownout"};
	for (uint8_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i) {
		test_reset_reason = esp_reset_reason_t(i);
		assert(strcmp(device_status::snapshot().reset, labels[i]) == 0);
	}
	std::cout << "Diagnostics platform pools, reset causes, uptime and read-only Wi-Fi passed\n";
	return 0;
}

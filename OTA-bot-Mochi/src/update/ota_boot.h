/**
 * @file ota_boot.h
 * @brief Defer OTA acceptance until setup and the main loop have remained healthy.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Main-task health window; an unhealthy poll restarts the entire observation interval. */
class OtaBootHealth {
	bool observing_ = false;
	uint32_t since_ = 0;
public:
	/** @brief Observe continuous health with rollover-safe timestamps.
	 * @param now Timestamp. @param healthy Setup and main-loop readiness. @return Fifteen seconds elapsed. */
	bool ready(uint32_t now, bool healthy) {
		if (!healthy) { observing_ = false; return false; }
		if (!observing_) { since_ = now; observing_ = true; }
		return uint32_t(now - since_) >= 15000;
	}
};

/** @brief Confirm only pending images after healthy setup and loop activity. @param now Timestamp. @param healthy Readiness. */
void ota_boot_poll(uint32_t now, bool healthy);

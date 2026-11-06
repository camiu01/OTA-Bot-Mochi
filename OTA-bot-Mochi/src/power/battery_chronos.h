/**
 * @file battery_chronos.h
 * @brief Main-task publication of the displayed battery estimate and PMU charge state to Chronos.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <ChronosESP32.h>
#include "battery.h"
#include "charging.h"

/** @brief Cache valid device battery data; Chronos sends changes and resends after reconnect.
 * @param watch Android companion transport. */
inline void battery_chronos_poll(ChronosESP32 &watch) {
	uint8_t percent = battery_percent();
	if (percent > 100) { return; }
	watch.setBattery(percent, charging_active());
}

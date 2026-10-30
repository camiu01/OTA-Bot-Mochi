/**
 * @file device_status.h
 * @brief Non-sensitive main-task diagnostics and atomic audio-worker observations.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino.h>
#include <atomic>
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_timer.h>
#endif

namespace device_status {
enum class SdState : uint8_t { Unchecked, Ready, Missing, Error };
inline std::atomic<SdState> sd{SdState::Unchecked};

/** @brief Runtime measurements; unavailable host values are never reported as zero free memory. */
struct Snapshot {
	uint64_t uptime = 0;
	uint32_t heap = 0, minimum_heap = 0, psram = 0;
	bool measured = false, wifi = false;
	const char *reset = "?";
};

/** @brief Collect platform counters without probing buses, storage or the network. @return Current measurements. */
inline Snapshot snapshot() {
	Snapshot value;
	value.uptime = millis();
#if defined(ARDUINO_ARCH_ESP32)
	value.uptime = uint64_t(esp_timer_get_time()) / 1000;
	value.measured = true;
	const uint32_t internal = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
	value.heap = heap_caps_get_free_size(internal);
	value.minimum_heap = heap_caps_get_minimum_free_size(internal);
	value.psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
	value.wifi = WiFi.status() == WL_CONNECTED;
	switch (esp_reset_reason()) {
		case ESP_RST_POWERON: value.reset = "Power on"; break;
		case ESP_RST_SW: value.reset = "Software"; break;
		case ESP_RST_PANIC: value.reset = "Panic"; break;
		case ESP_RST_INT_WDT: case ESP_RST_TASK_WDT: case ESP_RST_WDT: value.reset = "Watchdog"; break;
		case ESP_RST_DEEPSLEEP: value.reset = "Deep sleep"; break;
		case ESP_RST_BROWNOUT: value.reset = "Brownout"; break;
		default: value.reset = "Other"; break;
	}
#endif
	return value;
}
}

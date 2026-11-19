/**
 * @file obd_state.h
 * @brief Bounded vehicle snapshot shared by the BLE OBD worker and the drive view.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <string.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/** @brief Link progress shown on the OBD view. */
enum class ObdStatus : uint8_t { Idle, Scanning, Connecting, Starting, Live, NoEcu };

/** @brief Field bits in VehicleData::valid. */
enum ObdField : uint8_t { OBD_RPM = 1, OBD_SPEED = 2, OBD_COOLANT = 4, OBD_THROTTLE = 8, OBD_FUEL = 16, OBD_VOLTS = 32 };

struct VehicleData {
	ObdStatus status = ObdStatus::Idle;
	uint8_t valid = 0;	/**< ObdField bits received since the link started. */
	float rpm = 0, speed = 0, coolant = 0, throttle = 0, fuel = 0, volts = 0;	/**< rpm, km/h, C, %, %, V. */
	uint32_t updated = 0;	/**< millis() of the last decoded value. */
	char name[20] = {};	/**< Advertised dongle name, ASCII, bounded. */
};

namespace obd_detail {
inline StaticSemaphore_t &storage() { static StaticSemaphore_t value; return value; }
/** @brief Created by the main task before the worker starts. */
inline SemaphoreHandle_t lock() { static SemaphoreHandle_t value = xSemaphoreCreateMutexStatic(&storage()); return value; }
inline VehicleData &data() { static VehicleData value; return value; }
inline std::atomic<bool> &wanted() { static std::atomic<bool> value{false}; return value; }
}

/** @brief Copy the latest vehicle state for drawing. @return Snapshot. */
inline VehicleData obd_snapshot() {
	xSemaphoreTake(obd_detail::lock(), portMAX_DELAY);
	VehicleData copy = obd_detail::data();
	xSemaphoreGive(obd_detail::lock());
	return copy;
}
/** @brief Replace the shared state from the worker. @param value New state. */
inline void obd_publish(const VehicleData &value) {
	xSemaphoreTake(obd_detail::lock(), portMAX_DELAY);
	obd_detail::data() = value;
	xSemaphoreGive(obd_detail::lock());
}
/** @brief Main task: request scanning/polling while the drive page is visible. @param on Wanted. */
inline void obd_want(bool on) { (void)obd_detail::lock(); obd_detail::wanted() = on; }
/** @brief Worker: read the main task's request. @return Wanted. */
inline bool obd_wanted() { return obd_detail::wanted(); }

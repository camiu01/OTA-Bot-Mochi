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

/** @brief Selectable engine values; also the settings OBD slot choices (0 = hidden). */
enum ObdValue : uint8_t {
	OBD_V_NONE, OBD_V_COOLANT, OBD_V_THROTTLE, OBD_V_FUEL, OBD_V_BATTERY, OBD_V_INTAKE, OBD_V_LOAD, OBD_V_MAP,
	OBD_V_TIMING, OBD_V_STFT, OBD_V_LTFT, OBD_V_O2, OBD_V_ECU, OBD_V_MAF, OBD_V_AMBIENT, OBD_V_BARO, OBD_V_RUNTIME,
	OBD_VALUE_COUNT
};

/** @brief Fixed field bits in VehicleData::valid; selectable values use obd_bit(). */
enum ObdField : uint32_t { OBD_RPM = 1, OBD_SPEED = 2, OBD_MIL = 4 };
/** @brief Valid bit of a selectable value. @param value ObdValue. @return Bit. */
constexpr uint32_t obd_bit(uint8_t value) { return uint32_t(1) << (8 + value); }

struct VehicleData {
	ObdStatus status = ObdStatus::Idle;
	uint32_t valid = 0;	/**< ObdField and obd_bit() bits received since the link started. */
	float rpm = 0, speed = 0;	/**< rpm, km/h. */
	float value[OBD_VALUE_COUNT] = {};	/**< Decoded ObdValue readings in display units. */
	bool mil = false;	/**< Check-engine lamp on (PID 01, read-only). */
	uint8_t dtc = 0;	/**< Stored emission-related trouble codes reported by PID 01. */
	uint32_t updated = 0;	/**< millis() of the last decoded value. */
	char name[20] = {};	/**< Advertised dongle name, ASCII, bounded. */
};

namespace obd_detail {
inline StaticSemaphore_t &storage() { static StaticSemaphore_t value; return value; }
/** @brief Created by the main task before the worker starts. */
inline SemaphoreHandle_t lock() { static SemaphoreHandle_t value = xSemaphoreCreateMutexStatic(&storage()); return value; }
inline VehicleData &data() { static VehicleData value; return value; }
inline std::atomic<bool> &wanted() { static std::atomic<bool> value{false}; return value; }
/** @brief ObdValue bits the drive page shows; the worker polls only these. */
inline std::atomic<uint32_t> &values() { static std::atomic<uint32_t> value{0x1E}; return value; }
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
/**
 * @brief Main task: request scanning/polling while the drive page is visible.
 * @param on Wanted. @param values Bit per ObdValue to poll (1 << value).
 */
inline void obd_want(bool on, uint32_t values) { (void)obd_detail::lock(); obd_detail::values() = values; obd_detail::wanted() = on; }
/** @brief Worker: read the main task's request. @return Wanted. */
inline bool obd_wanted() { return obd_detail::wanted(); }
/** @brief Worker: values to poll. @return Bit per ObdValue. */
inline uint32_t obd_wanted_values() { return obd_detail::values(); }

/**
 * @file imu_calibration_store.h
 * @brief Separate, versioned NVS gravity reference; writes only after explicit calibration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "imu_orientation.h"
#include "../../pin_config.h"
#include <Preferences.h>
#include <string.h>

constexpr uint8_t IMU_CALIBRATION_BYTES = 9;
constexpr uint8_t IMU_CALIBRATION_BOARD = MOCHI_BOARD_LCD_169 ? 0 : 2;

/** @brief Validate and decode an exact-size record without modifying the destination on failure. @param bytes Record. @param length Byte count. @param policy Destination. @param manual Current manual flip. @return Valid reference. */
inline bool imu_calibration_decode(const uint8_t *bytes, size_t length, ImuOrientation &policy, bool manual) {
	if (!bytes || length != IMU_CALIBRATION_BYTES || bytes[0] != 1 ||
		(bytes[1] & ~1U) != IMU_CALIBRATION_BOARD) { return false; }
	uint8_t checksum = 0xA5;
	for (uint8_t i = 0; i < IMU_CALIBRATION_BYTES - 1; ++i) { checksum ^= bytes[i]; }
	if (checksum != bytes[8]) { return false; }
	float axes[3], energy = 0;
	for (uint8_t i = 0; i < 3; ++i) {
		uint16_t raw = uint16_t(bytes[2 + i * 2]) | (uint16_t(bytes[3 + i * 2]) << 8);
		int32_t signed_raw = raw >= 32768 ? int32_t(raw) - 65536 : raw;
		axes[i] = signed_raw / 16384.0f;
		energy += axes[i] * axes[i];
	}
	if (energy < 0.80f || energy > 1.22f) { return false; }
	policy.restore(axes, bytes[1] & 1, manual);
	return true;
}

/** @brief Read a saved reference, leaving policy unchanged when absent/corrupt/unavailable. @param policy Destination. @param manual Current manual flip. @return Loaded. */
inline bool imu_calibration_load(ImuOrientation &policy, bool manual) {
	Preferences storage;
	if (!storage.begin("mochi-imu", true)) { return false; }
	uint8_t bytes[IMU_CALIBRATION_BYTES] = {};
	size_t length = storage.getBytesLength("zero");
	bool read = length == sizeof(bytes) && storage.getBytes("zero", bytes, sizeof(bytes)) == sizeof(bytes);
	storage.end();
	return read && imu_calibration_decode(bytes, sizeof(bytes), policy, manual);
}

/** @brief Save one successful calibration; identical records do not wear flash. @param policy Calibrated state. @return Saved or already identical. */
inline bool imu_calibration_save(const ImuOrientation &policy) {
	if (policy.status != ImuCalibration::Ready) { return false; }
	uint8_t bytes[IMU_CALIBRATION_BYTES] = {1, uint8_t(IMU_CALIBRATION_BOARD | policy.reference_flipped())};
	for (uint8_t i = 0; i < 3; ++i) {
		float value = policy.gravity(i);
		if (!isfinite(value) || fabsf(value) > 1.11f) { return false; }
		uint16_t raw = uint16_t(int16_t(lroundf(value * 16384)));
		bytes[2 + i * 2] = raw & 255; bytes[3 + i * 2] = raw >> 8;
	}
	bytes[8] = 0xA5;
	for (uint8_t i = 0; i < 8; ++i) { bytes[8] ^= bytes[i]; }
	ImuOrientation validated;
	if (!imu_calibration_decode(bytes, sizeof(bytes), validated, false)) { return false; }
	Preferences storage;
	if (!storage.begin("mochi-imu", false)) { return false; }
	uint8_t previous[IMU_CALIBRATION_BYTES] = {};
	bool same = storage.getBytesLength("zero") == sizeof(previous) &&
		storage.getBytes("zero", previous, sizeof(previous)) == sizeof(previous) &&
		memcmp(previous, bytes, sizeof(bytes)) == 0;
	bool saved = same || storage.putBytes("zero", bytes, sizeof(bytes)) == sizeof(bytes);
	storage.end();
	return saved;
}

/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file eyes_imu.cpp
 * @brief QMI8658C accelerometer polling, tilt mapping and shake detection.
 */
#include "eyes_api.h"
#include "motion_filter.h"
#include "pickup_policy.h"
#include "imu_orientation.h"
#include "imu_calibration_store.h"
#include "eyes_faces.h"
#include "face_filter.h"
#include "../settings/settings.h"
#include "../bus/i2c_bus.h"
#include "../play/dice_input.h"
#include "../play/drive_meter.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <math.h>

#define QMI_REG_WHOAMI 0x00	/**< Chip ID register. */
#define QMI_REG_CTRL1 0x02	/**< Interface control register. */
#define QMI_REG_CTRL2 0x03	/**< Accelerometer range and sample rate. */
#define QMI_REG_CTRL3 0x04	/**< Gyroscope range and sample rate. */
#define QMI_REG_CTRL7 0x08	/**< Sensor enable register. */
#define QMI_REG_AX_L 0x35	/**< First accelerometer data register. */
#define QMI_POLL_MS 50	/**< Minimum interval between IMU reads. */

static bool s_imuOk = false;
static uint8_t s_address = 0x6B;
static uint32_t s_lastPoll = 0;
static bool s_flipped = false;
static bool s_gyro = false;	/**< Gyroscope powered, only while the drive dashboard is visible. */
static MotionFilter s_motion;
static PickupPolicy s_pickup;

/**
 * @brief Write one byte to a QMI8658C register.
 * @param reg Target register address.
 * @param val Value to write.
 * @return True on a successful acknowledged write.
 */
static bool imu_write(uint8_t reg, uint8_t val) {
	TwoWire &bus = i2c_bus();
	bus.beginTransmission(s_address);
	bus.write(reg);
	bus.write(val);
	return bus.endTransmission() == 0;
}

/**
 * @brief Read raw accelerometer, then optionally gyroscope, axes in one burst.
 * @param raw Output axes: accel X/Y/Z, then gyro X/Y/Z when count is 6.
 * @param count Three (accelerometer) or six (accelerometer and gyroscope).
 * @return True on a complete read, false on I2C error.
 */
static bool imu_read(int16_t *raw, uint8_t count) {
	TwoWire &bus = i2c_bus();
	bus.beginTransmission(s_address);
	bus.write((uint8_t)QMI_REG_AX_L);
	if (bus.endTransmission(false) != 0) {
		return false;
	}
	if (bus.requestFrom(s_address, uint8_t(count * 2)) != count * 2) {
		return false;
	}
	for (uint8_t i = 0; i < count; ++i) {
		uint8_t low = bus.read();
		raw[i] = (int16_t)((bus.read() << 8) | low);
	}
	return true;
}

/** @brief Power the gyroscope only for the drive dashboard. @param on Wanted. @return Gyro readings usable. */
static bool imu_gyro(bool on) {
	if (on != s_gyro && imu_write(QMI_REG_CTRL7, on ? 0x03 : 0x01)) { s_gyro = on; }
	return s_gyro;
}

/**
 * @brief Init QMI8658C on the shared board I2C bus at QMI_SDA/QMI_SCL.
 */
void eyes_imu_init() {
	s_imuOk = false;
	s_motion = MotionFilter{};
	s_pickup.reset();
	eyes_imu_set_tilt(0, 0);
	if (!i2c_bus_begin(QMI_SDA, QMI_SCL, 400000)) { return; }
	TwoWire &bus = i2c_bus();
	for (uint8_t address : {0x6B, 0x6A}) {
		s_address = address;
		bus.beginTransmission(s_address);
		bus.write((uint8_t)QMI_REG_WHOAMI);
		if (bus.endTransmission(false) != 0) { continue; }
		if (bus.requestFrom(s_address, (uint8_t)1) != 1 || bus.read() != 0x05) { continue; }
		// Auto-increment, +/-16g at 125 Hz (2048 LSB/g); gyro +/-512 dps (64 LSB/dps)
		// is configured but stays off until the drive dashboard enables it.
		s_imuOk = imu_write(QMI_REG_CTRL1, 0x40) && imu_write(QMI_REG_CTRL2, 0x36) &&
			imu_write(QMI_REG_CTRL3, 0x56) && imu_write(QMI_REG_CTRL7, 0x01);
		s_gyro = false;
		break;
	}
	s_lastPoll = millis();
	if (s_imuOk && imu_calibration_load(imu_orientation(), settings_get().flipped)) {
		float sign = imu_orientation().applied ? -1.0f : 1.0f;
		s_motion.calibrate(imu_orientation().gravity(0) * sign, imu_orientation().gravity(1) * sign, imu_orientation().gravity(2));
		s_flipped = imu_orientation().applied;
	}
}

/** @brief Report verified sensor initialization. @return Detected and configured successfully. */
bool eyes_imu_ready() { return s_imuOk; }

/** @brief Start a bounded stable calibration without blocking the main loop. @return IMU available. */
bool eyes_imu_calibrate() {
	if (!s_imuOk) { return false; }
	imu_orientation().manual_at_calibration = settings_get().flipped;
	imu_orientation().calibrate(millis(), imu_orientation().applied);
	return true;
}

/**
 * @brief Set pupil offset from tilt in pixels.
 * @param tx Horizontal offset.
 * @param ty Vertical offset.
 */
void eyes_imu_set_tilt(int16_t tx, int16_t ty) {
	EyeState &st = eyes_state();
	st.tiltX = tx;
	st.tiltY = ty;
}

/**
 * @brief Force DIZZY expression for 2s.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_trigger_shake(uint32_t nowMs) {
	if (!face_filter().allows(DIZZY)) { return; }
	EyeState &st = eyes_state();
	EyeConfig &cfg = eyes_config();
	st.wakeUntilMs = st.petUntilMs = 0;
	if (st.dizzyUntilMs != 0) {
		st.dizzyUntilMs = nowMs + 2000;
		return;
	}
	st.savedExpr = cfg.expression;
	cfg.expression = DIZZY;
	st.dizzyUntilMs = nowMs + 2000;
}

/**
 * @brief Poll accelerometer, drive tilt and shake-to-dizzy.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_poll(uint32_t nowMs) {
	imu_orientation().tick(nowMs);
	if (!s_imuOk) {
		return;
	}
	if ((uint32_t)(nowMs - s_lastPoll) < QMI_POLL_MS) {
		return;
	}
	s_lastPoll = nowMs;
	int16_t raw[6] = {};
	bool gyro = imu_gyro(drive_meter().active);
	if (!imu_read(raw, gyro ? 6 : 3)) {
		return;
	}
	float fx = (float)raw[0] / 2048.0f;
	float fy = (float)raw[1] / 2048.0f;
	float fz = (float)raw[2] / 2048.0f;
	auto &orientation = imu_orientation();
	orientation.target(settings_get().flipped);
	bool calibrated = orientation.sample(fx, fy, fz, nowMs);
	if (orientation.status == ImuCalibration::Sampling) {
		s_pickup.reset();
		eyes_imu_set_tilt(0, 0);
		return;
	}
	// A 180-degree mounting reverses the in-panel axes; re-baseline so gaze, mood
	// deltas, and shake keep reading in the user's frame instead of the board's.
	bool flipped = orientation.enabled && orientation.status == ImuCalibration::Ready ?
		orientation.applied : settings_get().flipped;
	if (flipped != s_flipped) {
		s_flipped = flipped;
		s_motion = MotionFilter{};
		s_pickup.reset();
	}
	if (calibrated) {
		s_pickup.reset();
		orientation.persisted = imu_calibration_save(orientation);
		orientation.save_failed = !orientation.persisted;
		float sign = flipped ? -1.0f : 1.0f;
		s_motion.calibrate(orientation.gravity(0) * sign, orientation.gravity(1) * sign, orientation.gravity(2));
	}
	if (flipped) {
		fx = -fx;
		fy = -fy;
	}
	if (drive_meter().active) {
		const float rad = 3.14159265f / 180.0f / 64.0f, sign = flipped ? -1.0f : 1.0f;
		const float a[3] = {fx, fy, fz}, w[3] = {sign * raw[3] * rad, sign * raw[4] * rad, raw[5] * rad};
		drive_meter().sample(a, w, nowMs);
	}
	MotionSample sample = s_motion.update(fx, fy, fz, nowMs,
		dice_input().active || (settings_get().shake && eyes_faces_automatic_ready(nowMs)));
	eyes_imu_set_tilt(sample.x, sample.y);
	if (s_pickup.update(fx, fy, fz, nowMs, settings_get().imu_faces)) { eyes_wake(nowMs); }
	eyes_faces_motion(sample.delta_x, sample.delta_y, sample.delta_z, nowMs);
	if (sample.shake) {
		if (dice_input().active) { dice_input().pending = true; }
		else { eyes_imu_trigger_shake(nowMs); }
	}
}

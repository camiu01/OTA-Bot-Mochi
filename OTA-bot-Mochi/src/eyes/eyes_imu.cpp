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
#include "eyes_faces.h"
#include "../settings/settings.h"
#include "../bus/i2c_bus.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <math.h>

#define QMI_REG_WHOAMI 0x00	/**< Chip ID register. */
#define QMI_REG_CTRL1 0x02	/**< Interface control register. */
#define QMI_REG_CTRL2 0x03	/**< Accelerometer range and sample rate. */
#define QMI_REG_CTRL7 0x08	/**< Sensor enable register. */
#define QMI_REG_AX_L 0x35	/**< First accelerometer data register. */
#define QMI_POLL_MS 50	/**< Minimum interval between IMU reads. */

static bool s_imuOk = false;
static uint8_t s_address = 0x6B;
static uint32_t s_lastPoll = 0;
static bool s_flipped = false;
static MotionFilter s_motion;

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
 * @brief Read raw accelerometer axes from the sensor.
 * @param ax Output raw X axis value.
 * @param ay Output raw Y axis value.
 * @param az Output raw Z axis value.
 * @return True on successful 6-byte read, false on I2C error.
 */
static bool imu_read_acc(int16_t *ax, int16_t *ay, int16_t *az) {
	TwoWire &bus = i2c_bus();
	bus.beginTransmission(s_address);
	bus.write((uint8_t)QMI_REG_AX_L);
	if (bus.endTransmission(false) != 0) {
		return false;
	}
	if (bus.requestFrom(s_address, (uint8_t)6) != 6) {
		return false;
	}
	uint8_t xl = bus.read();
	uint8_t xh = bus.read();
	uint8_t yl = bus.read();
	uint8_t yh = bus.read();
	uint8_t zl = bus.read();
	uint8_t zh = bus.read();
	*ax = (int16_t)((xh << 8) | xl);
	*ay = (int16_t)((yh << 8) | yl);
	*az = (int16_t)((zh << 8) | zl);
	return true;
}

/**
 * @brief Init QMI8658C on the shared board I2C bus at QMI_SDA/QMI_SCL.
 */
void eyes_imu_init() {
	s_imuOk = false;
	s_motion = MotionFilter{};
	eyes_imu_set_tilt(0, 0);
	if (!i2c_bus_begin(QMI_SDA, QMI_SCL, 400000)) { return; }
	TwoWire &bus = i2c_bus();
	for (uint8_t address : {0x6B, 0x6A}) {
		s_address = address;
		bus.beginTransmission(s_address);
		bus.write((uint8_t)QMI_REG_WHOAMI);
		if (bus.endTransmission(false) != 0) { continue; }
		if (bus.requestFrom(s_address, (uint8_t)1) != 1 || bus.read() != 0x05) { continue; }
		// Auto-increment, +/-16g at 125 Hz (2048 LSB/g), accelerometer only.
		s_imuOk = imu_write(QMI_REG_CTRL1, 0x40) &&
			imu_write(QMI_REG_CTRL2, 0x36) && imu_write(QMI_REG_CTRL7, 0x01);
		break;
	}
	s_lastPoll = millis();
}

/** @brief Report verified sensor initialization. @return Detected and configured successfully. */
bool eyes_imu_ready() { return s_imuOk; }

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
	EyeState &st = eyes_state();
	EyeConfig &cfg = eyes_config();
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
	if (!s_imuOk) {
		return;
	}
	if ((uint32_t)(nowMs - s_lastPoll) < QMI_POLL_MS) {
		return;
	}
	s_lastPoll = nowMs;
	int16_t ax = 0;
	int16_t ay = 0;
	int16_t az = 0;
	if (!imu_read_acc(&ax, &ay, &az)) {
		return;
	}
	float fx = (float)ax / 2048.0f;
	float fy = (float)ay / 2048.0f;
	float fz = (float)az / 2048.0f;
	// A 180-degree mounting reverses the in-panel axes; re-baseline so gaze, mood
	// deltas, and shake keep reading in the user's frame instead of the board's.
	bool flipped = settings_get().flipped;
	if (flipped != s_flipped) {
		s_flipped = flipped;
		s_motion = MotionFilter{};
	}
	if (flipped) {
		fx = -fx;
		fy = -fy;
	}
	MotionSample sample = s_motion.update(fx, fy, fz, nowMs, settings_get().shake);
	eyes_imu_set_tilt(sample.x, sample.y);
	eyes_faces_motion(sample.delta_x, sample.delta_y, sample.delta_z, nowMs);
	if (sample.shake) {
		eyes_imu_trigger_shake(nowMs);
	}
}

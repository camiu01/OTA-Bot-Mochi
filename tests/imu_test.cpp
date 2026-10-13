/**
 * @file imu_test.cpp
 * @brief Register-level native checks for QMI8658 initialization and scaling.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <Wire.h>
#include <assert.h>
#include <iostream>

/**
 * @brief Set a raw signed axis in the fake register bank.
 * @param reg Low-byte register.
 * @param value Raw reading.
 */
static void axis(uint8_t reg, int16_t value) {
	Wire.registers[0x6A][reg] = uint16_t(value) & 0xFF;
	Wire.registers[0x6A][reg + 1] = uint16_t(value) >> 8;
}

/** @brief Test absent sensors, identity, failed writes, and the explicit accelerometer range. */
static void detection_tests() {
	eyes_imu_init();
	assert(!eyes_imu_ready());
	Wire.present[0x6B] = true;
	Wire.registers[0x6B][0] = 0xFF;
	eyes_imu_init();
	assert(!eyes_imu_ready());
	Wire.registers[0x6B][0] = 0x05;
	Wire.fail_write_register = 3;
	eyes_imu_init();
	assert(!eyes_imu_ready());
	Wire.fail_write_register = -1;
	Wire.present[0x6B] = false;
	Wire.present[0x6A] = true;
	Wire.registers[0x6A][0] = 0x05;
	eyes_imu_init();
	assert(eyes_imu_ready());
	assert(Wire.registers[0x6A][2] == 0x40);
	assert(Wire.registers[0x6A][3] == 0x36);
	assert(Wire.registers[0x6A][8] == 0x01);
}

/** @brief Test scale, road-noise handling, shake opt-in, and restoration. @return Success. */
int main() {
	assert(settings_init());
	eyes_init(nullptr);
	settings_apply();
	EyeConfig &config = eyes_config();
	EyeState &state = eyes_state();
	detection_tests();
	axis(0x35, 0);
	axis(0x37, 0);
	axis(0x39, 2048);
	eyes_imu_poll(50);
	assert(state.tiltX == 0);
	axis(0x35, 2048);
	eyes_imu_poll(100);
	assert(state.tiltX > 0 && state.tiltY == 0 && state.dizzyUntilMs == 0);
	axis(0x35, 6144);
	eyes_imu_poll(150);
	assert(config.expression == NEUTRAL && state.dizzyUntilMs == 0);
	CompanionSettings profile = settings_get();
	profile.shake = true; profile.mood = 1;
	settings_update(profile);
	for (uint32_t now = 200; now <= 350; now += 50) {
		axis(0x35, ((now / 50) % 2) ? -6144 : 6144);
		eyes_imu_poll(now);
	}
	assert(config.expression == DIZZY && state.savedExpr == HAPPY && state.dizzyUntilMs == 2350);
	eyes_imu_poll(400);
	assert(state.dizzyUntilMs == 2350);
	Wire.read_ok = false;
	eyes_update(2350);
	assert(state.dizzyUntilMs == 0 && config.expression == HAPPY);
	assert(state.floatY >= -1 && state.floatY <= 1);
	std::cout << "IMU setup, mount-relative gaze, shake opt-in/cooldown, and restoration passed\n";
	return 0;
}

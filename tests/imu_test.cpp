/**
 * @file imu_test.cpp
 * @brief Register-level native checks for QMI8658 initialization and scaling.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include <Wire.h>
#include <assert.h>
#include <iostream>

/**
 * @brief Set a raw signed axis in the fake register bank.
 * @param reg Low-byte register.
 * @param value Raw reading.
 */
static void axis(uint8_t reg, int16_t value) {
	i2c_bus().registers[0x6A][reg] = uint16_t(value) & 0xFF;
	i2c_bus().registers[0x6A][reg + 1] = uint16_t(value) >> 8;
}

/** @brief Test absent sensors, identity, failed writes, and the explicit accelerometer range. */
static void detection_tests() {
	eyes_imu_init();
	assert(!eyes_imu_ready());
	i2c_bus().present[0x6B] = true;
	i2c_bus().registers[0x6B][0] = 0xFF;
	eyes_imu_init();
	assert(!eyes_imu_ready());
	i2c_bus().registers[0x6B][0] = 0x05;
	i2c_bus().fail_write_register = 3;
	eyes_imu_init();
	assert(!eyes_imu_ready());
	i2c_bus().fail_write_register = -1;
	i2c_bus().present[0x6B] = false;
	i2c_bus().present[0x6A] = true;
	i2c_bus().registers[0x6A][0] = 0x05;
	eyes_imu_init();
	assert(eyes_imu_ready());
	assert(i2c_bus().registers[0x6A][2] == 0x40);
	assert(i2c_bus().registers[0x6A][3] == 0x36);
	assert(i2c_bus().registers[0x6A][8] == 0x01);
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
	// The selected HAPPY face must finish its three visible seconds before shake can interrupt.
	state.effectClockInit = true; state.effectExpression = HAPPY; state.effectStartMs = 0;
	for (uint32_t now = 200; now <= 350; now += 50) {
		axis(0x35, ((now / 50) % 2) ? -6144 : 6144);
		eyes_imu_poll(now);
	}
	assert(config.expression == HAPPY && state.dizzyUntilMs == 0);
	for (uint32_t now = 3200; now <= 3350; now += 50) {
		axis(0x35, ((now / 50) % 2) ? -6144 : 6144);
		eyes_imu_poll(now);
	}
	assert(config.expression == DIZZY && state.savedExpr == HAPPY && state.dizzyUntilMs == 5350);
	eyes_imu_poll(3400);
	assert(state.dizzyUntilMs == 5350);
	i2c_bus().read_ok = false;
	eyes_update(5350);
	assert(state.dizzyUntilMs == 0 && config.expression == HAPPY);
	assert(state.floatY >= -1 && state.floatY <= 1);
	// A gentle sustained cornering load stays inside the old recenter window, so it used to
	// average itself back to centre; it must now keep the gaze displaced for the whole turn.
	i2c_bus().read_ok = true;
	axis(0x35, 246);
	for (uint32_t now = 10000; now <= 15000; now += 50) { eyes_imu_poll(now); }
	assert(state.tiltX > 0 && state.tiltY == 0);
	// A hard launch saturates instead of throwing the gaze out of its bounded socket.
	axis(0x35, 8192);
	for (uint32_t now = 15050; now <= 16000; now += 50) { eyes_imu_poll(now); }
	assert(state.tiltX == 14);
	axis(0x35, -8192);
	for (uint32_t now = 16050; now <= 17000; now += 50) { eyes_imu_poll(now); }
	assert(state.tiltX == -14);
	// A 180-degree mounting flips the panel axes, so the same physical lean still
	// reads toward the user's side after the filter re-baselines.
	CompanionSettings flipped = settings_get();
	flipped.flipped = true;
	assert(settings_update(flipped));
	axis(0x35, 0);
	axis(0x37, 0);
	axis(0x39, 2048);
	eyes_imu_poll(17050);
	axis(0x35, 8192);
	eyes_imu_poll(17100);
	eyes_imu_poll(17150);
	assert(state.tiltX == -14 && state.tiltY == 0);
	std::cout << "IMU setup, mount-relative gaze, shake opt-in/cooldown, and restoration passed\n";
	return 0;
}

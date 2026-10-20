/**
 * @file imu_test.cpp
 * @brief Register-level native checks for QMI8658 initialization and scaling.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/src/eyes/imu_orientation.h"
#include "../OTA-bot-Mochi/src/eyes/imu_calibration_store.h"
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

/** @brief Stable calibration, turnover debounce, invalid poses, manual override, and timestamp wrap. */
static void orientation_tests() {
	ImuOrientation p;
	assert(!p.target(false));
	p.calibrate(0, false);
	for (uint32_t now = 50; now <= 1000; now += 50) { p.sample(0, 1, 0, now); }
	assert(p.status == ImuCalibration::Ready && !p.target(false));
	for (uint32_t now = 1050; now < 1650; now += 50) { p.sample(0, -1, 0, now); }
	assert(!p.target(false));
	p.sample(0, -1, 0, 1650);
	assert(p.target(false));
	for (uint32_t now = 1700; now < 3000; now += 50) { p.sample(0, 0, 1, now); }
	assert(p.target(false));
	p.enabled = false; assert(!p.target(false));
	p.enabled = true;
	assert(p.target(true) && p.status == ImuCalibration::Idle);
	p.calibrate(3000, false);
	for (uint32_t now = 3050; now <= 8000; now += 50) { p.sample(0, now % 100 ? 1 : -1, 0, now); }
	assert(p.status == ImuCalibration::Failed);
	p.manual_at_calibration = false;
	p.calibrate(UINT32_MAX - 500, false);
	for (uint32_t i = 1; i <= 20; ++i) { p.sample(0, 1, 0, uint32_t(UINT32_MAX - 500 + i * 50)); }
	assert(p.status == ImuCalibration::Ready);
	for (uint32_t now = 550; now <= 1150; now += 50) { p.sample(0, -1, 0, now); }
	assert(p.target(false));
}

/** @brief Exercise save/reload, replacement, unchanged-write suppression, and storage failure isolation. */
static void calibration_storage_tests() {
	ImuOrientation p, loaded;
	test_nvs.clear();
	assert(!imu_calibration_load(loaded, false));
	assert(!imu_calibration_save(p));
	p.calibrate(0, true);
	for (uint32_t now = 50; now <= 1000; now += 50) { p.sample(0, -1, 0, now); }
	auto profile = test_profile;
	unsigned int writes = test_flash_writes;
	assert(imu_calibration_save(p) && test_flash_writes == writes + 1);
	assert(imu_calibration_save(p) && test_flash_writes == writes + 1);
	assert(test_profile == profile);
	assert(imu_calibration_load(loaded, false) && loaded.persisted && loaded.target(false));
	assert(loaded.gravity(1) == -1);
	p.calibrate(2000, false);
	for (uint32_t now = 2050; now <= 3000; now += 50) { p.sample(0.6f, 0.8f, 0, now); }
	test_save_fails = true;
	assert(!imu_calibration_save(p) && test_flash_writes == writes + 1);
	assert(imu_calibration_load(loaded, false) && loaded.gravity(1) == -1);
	test_save_fails = false;
	assert(imu_calibration_save(p) && test_flash_writes == writes + 2);
	assert(imu_calibration_load(loaded, true) && !loaded.target(true));
	assert(fabsf(loaded.gravity(0) - 0.6f) < 0.0001f && fabsf(loaded.gravity(1) - 0.8f) < 0.0001f);
	test_storage_ready = false;
	assert(!imu_calibration_load(loaded, false) && !imu_calibration_save(p));
	test_storage_ready = true;
	auto saved = test_nvs["mochi-imu/zero"];
	for (uint8_t byte : {uint8_t(0), uint8_t(1), uint8_t(3), uint8_t(8)}) {
		test_nvs["mochi-imu/zero"] = saved;
		test_nvs["mochi-imu/zero"][byte] ^= 0x80;
		assert(!imu_calibration_load(loaded, false));
	}
	uint8_t zero[IMU_CALIBRATION_BYTES] = {1, IMU_CALIBRATION_BOARD};
	zero[8] = uint8_t(0xA5 ^ 1 ^ IMU_CALIBRATION_BOARD);
	assert(!imu_calibration_decode(zero, sizeof(zero), loaded, false));
	assert(!imu_calibration_decode(saved.data(), saved.size() - 1, loaded, false));
	assert(!imu_calibration_decode(nullptr, saved.size(), loaded, false));
	test_nvs.clear();
}

/** @brief Test scale, road-noise handling, shake opt-in, and restoration. @return Success. */
int main() {
	assert(settings_init());
	orientation_tests();
	calibration_storage_tests();
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
	test_now = 18000;
	unsigned int writes = test_flash_writes;
	assert(eyes_imu_calibrate());
	for (uint32_t now = 18050; now <= 19000; now += 50) {
		axis(0x35, 0); axis(0x37, 2048); axis(0x39, 0);
		eyes_imu_poll(now);
	}
	assert(imu_orientation().status == ImuCalibration::Ready);
	assert(imu_orientation().persisted && !imu_orientation().save_failed);
	assert(test_flash_writes == writes + 1);
	auto saved_zero = test_nvs["mochi-imu/zero"];
	assert(state.tiltX == 0 && state.tiltY == 0);
	test_now = 20000;
	assert(eyes_imu_calibrate());
	i2c_bus().read_ok = false;
	eyes_imu_poll(25000);
	assert(imu_orientation().status == ImuCalibration::Failed);
	assert(test_nvs["mochi-imu/zero"] == saved_zero && test_flash_writes == writes + 1);
	i2c_bus().read_ok = true;
	eyes_imu_init(); // Simulate a reboot without discarding the NVS fake.
	assert(imu_orientation().status == ImuCalibration::Ready && imu_orientation().persisted);
	axis(0x35, 819);
	eyes_imu_poll(25050);
	assert(state.tiltX > 0); // Uses saved zero, not the first post-boot sample.
	test_now = 26000;
	assert(eyes_imu_calibrate());
	test_save_fails = true;
	for (uint32_t now = 26050; now <= 27000; now += 50) {
		axis(0x35, 0); axis(0x37, -2048);
		eyes_imu_poll(now);
	}
	assert(imu_orientation().status == ImuCalibration::Ready && imu_orientation().save_failed);
	assert(test_nvs["mochi-imu/zero"] == saved_zero && test_flash_writes == writes + 1);
	test_save_fails = false;
	std::cout << "IMU setup, mount-relative gaze, shake opt-in/cooldown, and restoration passed\n";
	return 0;
}

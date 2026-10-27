/**
 * @file power_button_test.cpp
 * @brief PMU registers, original LCD latch, hold/release policy and wake failures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/power_button.h"
#include "../OTA-bot-Mochi/src/power/power_button_policy.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <Arduino.h>
#include <cassert>
#include <iostream>
#if MOCHI_BOARD_LCD_169
#include <driver/gpio.h>
#include <esp_sleep.h>
#endif

/** @brief Ignore boot holds/bounce/short clicks, fire once after release, handle rollover. */
static void hold_tests() {
	PowerButtonHold p;
	p.begin(true, 0);
	assert(!p.update(false, 5000) && !p.update(false, 5030));
	assert(!p.update(true, 6000) && !p.update(true, 6030));
	assert(!p.update(false, 10000) && p.update(false, 10030));
	assert(!p.update(false, 10060));
	assert(!p.update(true, 11000) && !p.update(true, 11030));
	assert(!p.update(false, 11050) && !p.update(false, 11080));
	assert(!p.update(true, 12000) && !p.update(false, 12010));
	assert(!p.update(true, 12020) && !p.update(true, 12050));
	assert(!p.update(false, 16000) && !p.update(false, 16030));
	p.begin(false, UINT32_MAX - 100);
	assert(!p.update(true, UINT32_MAX - 70) && !p.update(true, UINT32_MAX - 40));
	assert(!p.update(false, 4050) && p.update(false, 4080));
}

#if defined(MOCHI_BOARD_AMOLED_18)
/** @brief Preserve PMU power/charging settings and fail closed on I2C errors. */
static void board_tests() {
	TwoWire &wire = i2c_bus();
	wire.begin_ok = false; assert(!power_button_init());
	wire.begin_ok = true; assert(!power_button_init());
	wire.present[0x34] = true;
	auto &registers = wire.registers[0x34];
	registers[0x22] = 0xA5; registers[0x27] = 0xDB;
	registers[0x18] = 0x16; registers[0x80] = 0x1F;
	wire.read_ok = false;
	assert(!power_button_init() && registers[0x22] == 0xA5 && registers[0x27] == 0xDB);
	wire.read_ok = true; wire.fail_write_register = 0x27;
	assert(!power_button_init() && registers[0x22] == 0xA5 && registers[0x27] == 0xDB);
	wire.fail_write_register = 0x22;
	assert(!power_button_init() && registers[0x22] == 0xA5 && registers[0x27] == 0xD2);
	wire.fail_write_register = -1;
	assert(power_button_init());
	assert(registers[0x22] == 0xA6 && registers[0x27] == 0xD2);
	assert(registers[0x18] == 0x16 && registers[0x80] == 0x1F);
	assert(power_button_init());
	assert(!power_button_poll(5000) && !power_button_prepare_sleep() && !power_button_sleep());
}
#else
/** @brief Keep power latched, use original PWR pin, and reject wake configuration errors. */
static void board_tests() {
	i2c_bus().begin_ok = false;
	test_gpio_levels[SYS_OUT] = HIGH;
	assert(power_button_init() && test_gpio_levels[SYS_EN] == HIGH);
	assert(test_gpio_writes[SYS_EN] == 1);
	test_gpio_levels[SYS_OUT] = LOW;
	assert(!power_button_prepare_sleep() && test_gpio_wake_pin == -1);
	assert(!power_button_poll(100) && !power_button_poll(130));
	test_gpio_levels[SYS_OUT] = HIGH;
	assert(!power_button_poll(4100) && power_button_poll(4130));
	test_gpio_wake_error = 1;
	assert(!power_button_prepare_sleep() && test_gpio_levels[SYS_EN] == HIGH);
	test_gpio_wake_error = 0; test_sleep_enable_error = 1;
	assert(!power_button_prepare_sleep() && test_gpio_wake_pin == -1);
	test_sleep_enable_error = 0;
	assert(power_button_prepare_sleep() && test_gpio_wake_pin == SYS_OUT);
	assert(power_button_sleep() && test_gpio_levels[SYS_EN] == LOW && test_sleep_calls == 1);
	test_sleep_error = 1;
	assert(!power_button_sleep() && test_sleep_calls == 2);
}
#endif

/** @brief Run register, GPIO and deterministic policy checks. */
int main() {
	hold_tests();
	board_tests();
	std::cout << "PWR configuration, hold/release, rollover and wake failures passed\n";
}

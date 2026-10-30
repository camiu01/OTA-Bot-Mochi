/**
 * @file power_button_test.cpp
 * @brief PMU registers, original LCD latch, hold/release policy and wake failures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/power_button.h"
#include "../OTA-bot-Mochi/src/power/power_button_policy.h"
#include "../OTA-bot-Mochi/src/ui/boot_button.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <Arduino.h>
#include <cassert>
#include <iostream>
#if MOCHI_BOARD_LCD_169
#include <driver/gpio.h>
#include <esp_sleep.h>
#endif

/** @brief One BOOT release per page, no bounce/repeats/boot hold or blocked click, rollover safe. */
static void boot_button_tests() {
	BootPageButton button;
#if defined(MOCHI_BOARD_AMOLED_18)
	test_gpio_levels[BOOT_BUTTON] = LOW;
	button.begin(0);
	auto poll = [&](int level, uint32_t now, UiPage page = UiPage::Eyes, bool blocked = false) {
		test_gpio_levels[BOOT_BUTTON] = level;
		return button.poll(now, page, blocked);
	};
	assert(poll(HIGH, 100) == UiPage::Eyes && poll(HIGH, 130) == UiPage::Eyes);
	assert(poll(LOW, 200) == UiPage::Eyes && poll(HIGH, 210) == UiPage::Eyes);
	assert(poll(LOW, 220) == UiPage::Eyes && poll(LOW, 250) == UiPage::Eyes);
	assert(poll(LOW, 5000) == UiPage::Eyes);
	assert(poll(HIGH, 5010) == UiPage::Eyes && poll(HIGH, 5040) == UiPage::Clock);
	assert(poll(HIGH, 5070, UiPage::Clock) == UiPage::Clock);
	for (uint8_t i = 0; i < 7; ++i) {
		uint32_t now = 6000 + i * 200;
		UiPage page = static_cast<UiPage>(i);
		assert(poll(LOW, now, page) == page && poll(LOW, now + 30, page) == page);
		assert(poll(HIGH, now + 60, page) == page);
		assert(poll(HIGH, now + 90, page) == static_cast<UiPage>((i + 1) % 7));
	}
	assert(poll(LOW, 8000) == UiPage::Eyes && poll(LOW, 8030) == UiPage::Eyes);
	assert(poll(LOW, 8040, UiPage::Eyes, true) == UiPage::Eyes);
	assert(poll(HIGH, 8100) == UiPage::Eyes && poll(HIGH, 8130) == UiPage::Eyes);
	assert(poll(LOW, 8200, UiPage::Eyes, true) == UiPage::Eyes);
	assert(poll(LOW, 8230, UiPage::Eyes, true) == UiPage::Eyes);
	assert(poll(HIGH, 8300) == UiPage::Eyes && poll(HIGH, 8330) == UiPage::Eyes);
	assert(poll(LOW, UINT32_MAX - 70) == UiPage::Eyes);
	assert(poll(LOW, UINT32_MAX - 40) == UiPage::Eyes);
	assert(poll(HIGH, 10, UiPage::Pong) == UiPage::Pong);
	assert(poll(HIGH, 40, UiPage::Pong) == UiPage::Music);
#else
	button.begin(0);
	assert(button.poll(5000, UiPage::Eyes, false) == UiPage::Eyes);
#endif
}

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
	assert(!power_button_init() && registers[0x22] == 0xA5 && registers[0x27] == 0xC2);
	wire.fail_write_register = -1;
	registers[0x41] = 0xA0; registers[0x49] = 0xF7;
	assert(power_button_init());
	assert(registers[0x22] == 0xA6 && registers[0x27] == 0xC2);
	assert(registers[0x41] == 0xA7 && registers[0x49] == 0xF0);
	assert(registers[0x18] == 0x16 && registers[0x80] == 0x1F);
	assert(power_button_init());
	assert(!power_button_poll(5000) && !power_button_prepare_sleep() && !power_button_sleep());
}

/** @brief Ignore boot/stale holds, preview fresh holds and cancel on release, including I2C failures. */
static void preview_tests() {
	TwoWire &wire = i2c_bus();
	auto &registers = wire.registers[0x34];
	registers[0x49] = 0x04;
	assert(!power_button_preview(40) && registers[0x49] == 0);
	registers[0x49] = 0x02;
	assert(!power_button_preview(80));
	registers[0x49] = 0x04;
	assert(!power_button_preview(100) && registers[0x49] == 0x04);
	assert(power_button_preview(120));
	assert(power_button_preview(160));
	registers[0x49] = 0x81;
	wire.read_ok = false;
	assert(!power_button_preview(200) && registers[0x49] == 0x81);
	wire.read_ok = true; wire.fail_write_register = 0x49;
	assert(!power_button_preview(240) && registers[0x49] == 0x81);
	wire.fail_write_register = -1;
	assert(!power_button_preview(280) && registers[0x49] == 0x80);
	registers[0x49] = 0x07;
	assert(!power_button_preview(320));
	registers[0x49] = 0x06;
	assert(power_button_preview(360));
	wire.fail_write_register = 0x49; registers[0x49] = 0x01;
	assert(!power_button_preview(400));
	wire.fail_write_register = -1;
	assert(!power_button_preview(440));
	registers[0x49] = 0x06;
	assert(power_button_preview(UINT32_MAX - 20));
	registers[0x49] = 0x01;
	assert(!power_button_preview(20));
	wire.fail_write_register = 0x41;
	assert(!power_button_init() && !power_button_preview(100));
	wire.fail_write_register = 0x49;
	assert(!power_button_init() && !power_button_preview(140));
	wire.fail_write_register = -1;
	assert(power_button_init());
}
#else
/** @brief Keep power latched, use original PWR pin, and reject wake configuration errors. */
static void board_tests() {
	i2c_bus().begin_ok = false;
	test_gpio_levels[SYS_OUT] = HIGH;
	assert(power_button_init() && test_gpio_levels[SYS_EN] == HIGH);
	assert(!power_button_preview(5000));
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
	boot_button_tests();
	board_tests();
#if defined(MOCHI_BOARD_AMOLED_18)
	preview_tests();
#endif
	std::cout << "PWR configuration/preview, BOOT page clicks, rollover and wake failures passed\n";
}

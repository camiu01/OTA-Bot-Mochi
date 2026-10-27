/**
 * @file gpio.h
 * @brief Native GPIO wake configuration model, not electrical validation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
using gpio_num_t = int;
constexpr int GPIO_INTR_LOW_LEVEL = 4;
inline int test_gpio_wake_pin = -1;
inline int test_gpio_wake_error = 0;
inline int gpio_wakeup_enable(gpio_num_t pin, int) {
	if (test_gpio_wake_error) { return test_gpio_wake_error; }
	test_gpio_wake_pin = pin;
	return 0;
}
inline int gpio_wakeup_disable(gpio_num_t) { test_gpio_wake_pin = -1; return 0; }

/**
 * @file esp_sleep.h
 * @brief Native sleep API outcomes, not power consumption or wake proof.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
constexpr int ESP_OK = 0;
inline int test_sleep_enable_error = 0, test_sleep_error = 0;
inline unsigned test_sleep_calls = 0;
inline int esp_sleep_enable_gpio_wakeup() { return test_sleep_enable_error; }
inline int esp_light_sleep_start() { ++test_sleep_calls; return test_sleep_error; }

/**
 * @file power_button_lcd.cpp
 * @brief Original LCD 1.69 power latch and USB-powered light-sleep fallback.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "power_button.h"
#include "../../pin_config.h"

#if MOCHI_BOARD_LCD_169
#include "power_button_policy.h"
#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

namespace {
PowerButtonHold button;
}

bool power_button_init() {
	// Original schematic: GPIO35 drives the SS8050/AO3401 battery-power latch.
	pinMode(SYS_EN, OUTPUT);
	digitalWrite(SYS_EN, HIGH);
	pinMode(SYS_OUT, INPUT_PULLUP);
	button.begin(digitalRead(SYS_OUT) == LOW, millis());
	return true;
}

bool power_button_poll(uint32_t now) {
	return button.update(digitalRead(SYS_OUT) == LOW, now);
}

bool power_button_prepare_sleep() {
	if (digitalRead(SYS_OUT) == LOW) { return false; }
	if (gpio_wakeup_enable(gpio_num_t(SYS_OUT), GPIO_INTR_LOW_LEVEL) != ESP_OK) { return false; }
	if (esp_sleep_enable_gpio_wakeup() == ESP_OK) { return true; }
	(void)gpio_wakeup_disable(gpio_num_t(SYS_OUT));
	return false;
}

bool power_button_sleep() {
	digitalWrite(SYS_EN, LOW);
	// USB bypasses the battery latch. GPIO36 is not RTC-capable on ESP32-S3,
	// so use GPIO light-sleep wake rather than an impossible deep-sleep wake.
	return esp_light_sleep_start() == ESP_OK;
}
#endif

/**
 * @file pwm_test.cpp
 * @brief Real backlight/buzzer adapters against shared-timer LEDC mocks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/backlight.h"
#include "../OTA-bot-Mochi/src/sound/buzzer_output.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <Arduino.h>
#include <assert.h>
#include <iostream>
/** @brief Tone frequency must not retune backlight; failures must not force buzzer HIGH. @return Success. */
int main() {
	backlight_init(0);
	assert(test_pwm[0].frequency == 5000 && test_pwm[0].duty == 76);
	assert(buzzer_begin() && test_pwm_pins[BUZZER] == 2);
	assert(test_pwm[0].timer != test_pwm[2].timer);
	for (uint16_t hz : {900, 1500, 3000}) {
		assert(buzzer_write(hz, 10));
		assert(test_pwm[0].frequency == 5000 && test_pwm[0].duty == 76);
		assert(test_pwm[2].frequency == hz && test_pwm[2].duty == 102);
	}
	assert(!buzzer_write(299, 10) && !buzzer_write(4001, 10) && !buzzer_write(2000, 31));
	backlight_suspend();
	assert(test_pwm[0].duty == 0 && test_pwm[2].duty == 102);
	assert(backlight_brightness() == 30);
	backlight_init(0);
	assert(test_pwm[0].duty == 76);
	buzzer_end();
	assert(test_pwm_pins[BUZZER] == -1 && test_pwm[2].duty == 0);
	uint32_t before = test_gpio_writes[BUZZER];
	test_pwm_fail = true;
	assert(!buzzer_begin() && test_gpio_writes[BUZZER] == before);
	backlight_init(0);
	backlight_suspend();
	assert(test_gpio_levels[LCD_BL] == LOW);
	std::cout << "ESP32 core " << ESP_ARDUINO_VERSION_MAJOR << " PWM timer isolation and silent failure passed\n";
}

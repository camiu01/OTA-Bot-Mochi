/**
 * @file buzzer_output.cpp
 * @brief Separate-timer LEDC output for the user-confirmed PWM/tone pin.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "buzzer_output.h"
#include "../power/pwm_channels.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <esp_arduino_version.h>
static bool ready = false;
static uint16_t frequency = 2200;

/** @brief Allocate audio PWM, initially silent. @return Success. */
bool buzzer_begin() {
	if (ready) { return true; }
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ready = ledcAttachChannel(BUZZER, 2200, 10, BUZZER_PWM_CHANNEL);
#else
	ready = ledcSetup(BUZZER_PWM_CHANNEL, 2200, 10) > 0;
	if (ready) { ledcAttachPin(BUZZER, BUZZER_PWM_CHANNEL); }
#endif
	frequency = 2200;
	if (ready) { buzzer_write(0, 0); }
	return ready;
}

/** @brief Change only audio's timer; never fall back to GPIO HIGH. @param hz Hertz. @param percent Duty. @return Success. */
bool buzzer_write(uint16_t hz, uint8_t percent) {
	if (!ready || percent > 30 || (hz && (hz < 300 || hz > 4000))) { return false; }
	if (hz && percent && frequency != hz) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
		if (!ledcChangeFrequency(BUZZER, hz, 10)) { return false; }
#else
		if (!ledcSetup(BUZZER_PWM_CHANNEL, hz, 10)) { return false; }
#endif
		frequency = hz;
	}
	uint32_t duty = hz ? uint32_t(percent) * 1023 / 100 : 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	return ledcWrite(BUZZER, duty);
#else
	ledcWrite(BUZZER_PWM_CHANNEL, duty);
	return true;
#endif
}

/** @brief Stop before detach; disabled-by-default builds do not touch the buzzer. */
void buzzer_end() {
	if (!ready) { return; }
	buzzer_write(0, 0);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ledcDetach(BUZZER);
#else
	ledcDetachPin(BUZZER);
#endif
	pinMode(BUZZER, OUTPUT);
	digitalWrite(BUZZER, LOW);
	ready = false;
}

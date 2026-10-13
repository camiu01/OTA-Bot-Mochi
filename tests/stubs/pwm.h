/**
 * @file pwm.h
 * @brief Native LEDC allocation/duty model, including shared-timer frequency changes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "esp_arduino_version.h"
#include <array>
#include <stdint.h>
#define OUTPUT 1
#define HIGH 1
#define LOW 0
struct FakePwm { uint32_t frequency = 0, duty = 0; uint8_t resolution = 0, timer = 0; bool attached = false; };
inline std::array<FakePwm, 8> test_pwm;
inline std::array<int8_t, 50> test_pwm_pins = [] { std::array<int8_t, 50> pins{}; pins.fill(-1); return pins; }();
inline std::array<uint32_t, 50> test_gpio_writes{};
inline bool test_pwm_fail = false;
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t pin, uint8_t) { if (pin < 50) { ++test_gpio_writes[pin]; } }
inline bool ledcAttachChannel(uint8_t pin, uint32_t hz, uint8_t bits, uint8_t channel) {
	if (test_pwm_fail || pin >= 50 || channel >= 8) { return false; }
	bool used[4] = {};
	int timer = -1;
	for (const auto &ch : test_pwm) {
		if (ch.attached) {
			used[ch.timer] = true;
			if (ch.frequency == hz && ch.resolution == bits) { timer = ch.timer; }
		}
	}
	if (timer < 0) { for (int i = 0; i < 4; ++i) { if (!used[i]) { timer = i; break; } } }
	if (timer < 0) { return false; }
	test_pwm[channel] = {hz, 0, bits, uint8_t(timer), true};
	test_pwm_pins[pin] = channel;
	return true;
}
inline bool ledcWrite(uint8_t target, uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	int ch = target < 50 ? test_pwm_pins[target] : -1;
#else
	int ch = target < 8 ? target : -1;
#endif
	if (test_pwm_fail || ch < 0 || !test_pwm[ch].attached) { return false; }
	test_pwm[ch].duty = duty; return true;
}
inline uint32_t ledcChangeFrequency(uint8_t pin, uint32_t hz, uint8_t bits) {
	int ch = pin < 50 ? test_pwm_pins[pin] : -1;
	if (test_pwm_fail || ch < 0) { return 0; }
	uint8_t timer = test_pwm[ch].timer;
	for (auto &item : test_pwm) { if (item.attached && item.timer == timer) { item.frequency = hz; item.resolution = bits; } }
	return hz;
}
inline bool ledcDetach(uint8_t pin) {
	if (pin >= 50 || test_pwm_pins[pin] < 0) { return false; }
	test_pwm[test_pwm_pins[pin]].attached = false; test_pwm_pins[pin] = -1; return true;
}
inline double ledcSetup(uint8_t ch, uint32_t hz, uint8_t bits) {
	if (test_pwm_fail || ch >= 8) { return 0; }
	test_pwm[ch].frequency = hz; test_pwm[ch].resolution = bits; test_pwm[ch].timer = ch / 2;
	return hz;
}
inline void ledcAttachPin(uint8_t pin, uint8_t ch) { if (pin < 50 && ch < 8) { test_pwm_pins[pin] = ch; test_pwm[ch].attached = true; } }
inline void ledcDetachPin(uint8_t pin) { ledcDetach(pin); }

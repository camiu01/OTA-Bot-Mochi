/**
 * @file backlight.cpp
 * @brief ESP32 LEDC brightness control with a safe GPIO fallback.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "backlight.h"
#include "brightness_policy.h"
#include "pwm_channels.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <esp_arduino_version.h>

namespace {
BrightnessPolicy policy;
bool pwm_ready = false;
uint8_t applied = 255;
void (*display_output)(uint8_t) = nullptr;

/** @brief Write a changed duty cycle. @param percent Brightness percentage. */
void apply(uint8_t percent) {
	if (!pwm_ready || applied == percent) { return; }
	applied = percent;
	if (display_output) { display_output(percent); return; }
#if LCD_BL >= 0
	uint32_t duty = uint32_t(percent) * 255 / 100;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ledcWrite(LCD_BL, duty);
#else
	ledcWrite(0, duty);
#endif
#endif
}
}

/** @brief Route brightness to a display controller instead of a backlight GPIO. @param fn Callback, or nullptr to unbind. */
void backlight_bind_display(void (*fn)(uint8_t percent)) {
	display_output = fn;
	if (fn) { pwm_ready = true; }
}

/** @brief Initialize the backlight channel. @param now Timestamp. */
void backlight_init(uint32_t now) {
	policy.last_activity = now;
#if LCD_BL >= 0
	pinMode(LCD_BL, OUTPUT);
	digitalWrite(LCD_BL, HIGH);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	pwm_ready = ledcAttachChannel(LCD_BL, 5000, 8, BACKLIGHT_PWM_CHANNEL);
#else
	pwm_ready = ledcSetup(0, 5000, 8) > 0;
	if (pwm_ready) { ledcAttachPin(LCD_BL, 0); }
#endif
	if (!pwm_ready) {
		digitalWrite(LCD_BL, HIGH);
		Serial.println("Backlight PWM unavailable; using full brightness");
	}
#else
	if (!pwm_ready) {
		Serial.println("No backlight pin or display hook; using full brightness");
	}
#endif
	apply(policy.brightness);
}

/** @brief Wake the screen after a phone event. @param now Timestamp. */
void backlight_activity(uint32_t now) { policy.last_activity = now; }

/**
 * @brief Update PWM using the pure touch and idle policy.
 * @param now Timestamp.
 * @param touching Finger contact.
 * @param keep_awake Critical UI data.
 */
void backlight_update(uint32_t now, bool touching, bool keep_awake) {
	policy.update(now, touching, keep_awake);
	apply(policy.output(now));
}

/** @brief Apply validated comfort settings immediately. @param percent Brightness. @param dim Idle policy. */
void backlight_configure(uint8_t percent, bool dim) {
	if (percent < 1 || percent > 90) { return; }
	policy.brightness = percent;
	policy.dim = dim;
	policy.last_activity = millis();
	apply(percent);
}

/** @brief Report configured brightness. @return Percent, or 100 when PWM failed. */
uint8_t backlight_brightness() { return pwm_ready ? policy.brightness : 100; }

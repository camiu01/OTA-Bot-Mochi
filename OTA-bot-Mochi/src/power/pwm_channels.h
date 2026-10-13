/**
 * @file pwm_channels.h
 * @brief Owned PWM channels use distinct timers and different initial frequency/resolution.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
constexpr uint8_t BACKLIGHT_PWM_CHANNEL = 0;
constexpr uint8_t BUZZER_PWM_CHANNEL = 2;
static_assert(BACKLIGHT_PWM_CHANNEL / 2 != BUZZER_PWM_CHANNEL / 2, "Independent legacy LEDC timers required");

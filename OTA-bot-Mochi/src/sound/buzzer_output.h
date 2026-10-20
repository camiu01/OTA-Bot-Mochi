/**
 * @file buzzer_output.h
 * @brief Lazy board audio output: LCD PWM or AMOLED ES8311/I2S.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
/** @brief Initialize the board's audio backend, initially silent. @return Success. */
bool buzzer_begin();
/** @brief Emit a bounded tone or silence. @param hz Hertz. @param percent PWM duty or PCM amplitude 0..30. @return Success. */
bool buzzer_write(uint16_t hz, uint8_t percent);
/** @brief Stop/mute only if previously initialized; LCD PWM is detached. */
void buzzer_end();
/** @brief Observe asynchronous I2S transport errors; PWM errors are synchronous. @return Healthy. */
bool buzzer_healthy();

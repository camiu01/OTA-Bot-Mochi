/**
 * @file buzzer_output.h
 * @brief Lazy PWM/tone output; no GPIO access while sound has never been enabled.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
/** @brief Attach existing buzzer pin to its owned PWM timer. @return Success. */
bool buzzer_begin();
/** @brief Emit frequency and bounded duty, or silence. @param hz Hertz. @param percent Duty 0..30. @return Success. */
bool buzzer_write(uint16_t hz, uint8_t percent);
/** @brief Stop and detach only if previously attached. */
void buzzer_end();

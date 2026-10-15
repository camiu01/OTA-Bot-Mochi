/**
 * @file backlight.h
 * @brief PWM backlight without sleep or unverified battery sensing.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Initialize PWM or fall back to full brightness. @param now Timestamp. */
void backlight_init(uint32_t now);
/**
 * @brief Route brightness to a display controller instead of a backlight GPIO.
 *
 * @details Boards without a backlight pad (AMOLED) bind a callback that maps
 *          the shared brightness percent onto the controller command; the
 *          policy, dimming, and wake behavior stay identical.
 * @param fn Callback receiving 1..90 percent, or nullptr to unbind.
 */
void backlight_bind_display(void (*fn)(uint8_t percent));
/** @brief Restart the idle timer. @param now Timestamp. */
void backlight_activity(uint32_t now);
/**
 * @brief Apply idle dimming and touch wake.
 * @param now Timestamp.
 * @param touching Finger contact.
 * @param keep_awake Active route or call.
 */
void backlight_update(uint32_t now, bool touching, bool keep_awake);
/** @brief Apply profile brightness. @param percent Validated 1..90 percent. @param dim Idle dimming. */
void backlight_configure(uint8_t percent, bool dim);
/** @brief Read the selected brightness. @return Percent brightness. */
uint8_t backlight_brightness();

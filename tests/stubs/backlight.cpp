/**
 * @file backlight.cpp
 * @brief Native brightness output substitute; the actual policy is tested separately.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../../OTA-bot-Mochi/src/power/backlight.h"
static uint8_t brightness = 30;
/** @brief Cache selected brightness. @param percent Percent. @param dim Unused output policy. */
void backlight_configure(uint8_t percent, bool dim) { brightness = percent; (void)dim; }
/** @brief Read test brightness. @return Selected percent. */
uint8_t backlight_brightness() { return brightness; }

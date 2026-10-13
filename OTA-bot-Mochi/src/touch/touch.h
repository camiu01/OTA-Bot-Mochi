/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <stdint.h>

/** @brief Initialize the touch controller on its dedicated I2C bus. */
void touch_init();

/**
 * @brief Poll touch coordinates without blocking the animation schedule.
 * @param now Timestamp in milliseconds.
 * @return +1 for the next screen, -1 for the previous screen, zero otherwise.
 */
int8_t touch_poll(uint32_t now);

/** @brief Read the last valid touch contact. @return True while a finger is held down. */
bool touch_pressed();

/** @brief Consume one short tap. @param x Output X. @param y Output Y. @return Tap available. */
bool touch_take_tap(int16_t &x, int16_t &y);
/** @brief Consume one stationary 1.2-second hold. @return Hold available. */
bool touch_take_hold();
/** @brief Consume one vertical gesture: +1 up, -1 down. @return Scroll direction. */
int8_t touch_take_scroll();

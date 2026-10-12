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

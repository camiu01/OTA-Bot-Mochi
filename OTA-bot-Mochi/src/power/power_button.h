/**
 * @file power_button.h
 * @brief Board-specific PWR-key setup, shutdown requests and wake support.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/**
 * @brief Configure AMOLED PMU timing or assert the original LCD power latch.
 * @return True on successful configuration.
 * @note Does not reprogram regulator voltages, battery charging or saved preferences.
 */
bool power_button_init();
/** @brief LCD: request shutdown on release after a four-second hold. AMOLED: no-op. */
bool power_button_poll(uint32_t now);
/** @brief LCD: validate the released key and arm GPIO light-sleep wake. */
bool power_button_prepare_sleep();
/** @brief LCD: drop battery hold and sleep if USB still supplies power; returns on wake/error. */
bool power_button_sleep();

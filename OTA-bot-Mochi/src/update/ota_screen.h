/**
 * @file ota_screen.h
 * @brief Maintenance screen, kept separate from the normal page carousel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>
/** @brief Silence audio and open the maintenance screen. @param gfx Canvas. @param now Timestamp. */
void ota_screen_begin(Arduino_GFX *gfx, uint32_t now, bool faces = false);
/** @brief Service maintenance touch, HTTP, display, and timeout. @param now Timestamp. */
void ota_screen_poll(uint32_t now);

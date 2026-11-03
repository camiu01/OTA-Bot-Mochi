/**
 * @file sd_update_screen.h
 * @brief Local microSD firmware preview, explicit confirmation and progress.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>
/** @brief Open the manual update screen. @param gfx Canvas. @param now Milliseconds. */
void sd_update_screen_begin(Arduino_GFX *gfx, uint32_t now);
/** @brief Service update work, touch and bounded display refresh. @param now Milliseconds. */
void sd_update_screen_poll(uint32_t now);

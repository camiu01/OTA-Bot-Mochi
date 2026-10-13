/**
 * @file clock_ui.h
 * @brief Synchronized clock rendering and invalidation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <Arduino_GFX_Library.h>
#include <ChronosESP32.h>

/** @brief Record the first completed phone time synchronization. */
void clock_time_synced();
/** @brief Check whether phone time was synchronized since boot. @return Valid local time. */
bool clock_ready();

/** @brief Redraw all clock fields after entering the screen. */
void clock_invalidate();

/**
 * @brief Update changed clock fields without clearing the header.
 * @param gfx Display canvas.
 * @param watch Synchronized Chronos clock.
 * @param connected Current Bluetooth connection state.
 * @return True if clock content changed.
 */
bool clock_draw(Arduino_GFX *gfx, ChronosESP32 &watch, bool connected);

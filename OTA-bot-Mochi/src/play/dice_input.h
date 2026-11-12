/**
 * @file dice_input.h
 * @brief Main-task shake handoff, enabled only while the dice screen is visible.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
struct DiceInput { bool active = false, pending = false; };
/** @brief Shared between IMU polling and screen dispatch, never BLE callbacks. */
inline DiceInput &dice_input() { static DiceInput input; return input; }

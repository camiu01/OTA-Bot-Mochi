/**
 * @file settings_menu.h
 * @brief Parked-use touch controls for appearance and dashboard comfort.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>

enum class MenuAction { None, Changed, Exit };
/** @brief Draw the selected settings tab. @param gfx Canvas. */
void settings_menu_draw(Arduino_GFX *gfx);
/** @brief Process a released tap. @param x X. @param y Y. @return Menu action. */
MenuAction settings_menu_tap(int16_t x, int16_t y);

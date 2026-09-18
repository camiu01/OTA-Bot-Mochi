/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file gfx_helper.cpp
 * @brief Text alignment helpers for Arduino_GFX display.
 */
#include "gfx_helper.h"

/**
 * @brief Calculate X position to horizontally center text.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @return X coordinate for centered placement.
 */
int16_t get_align_horizontal(Arduino_GFX *gfx, String str) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return (LCD_WIDTH - w) / 2;
}

/**
 * @brief Calculate Y position to vertically center text.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @return Y coordinate (baseline) for centered placement.
 */
int16_t get_align_vertical(Arduino_GFX *gfx, String str) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return (LCD_HEIGHT + h) / 2;
}

/**
 * @brief Draw text horizontally centered.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_horizontal(Arduino_GFX *gfx, String str, int16_t y) {
  int16_t x = get_align_horizontal(gfx, str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text vertically centered.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_vertical(Arduino_GFX *gfx, String str, int16_t x) {
  int16_t y = get_align_vertical(gfx, str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text centered both horizontally and vertically.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 */
void draw_align_center(Arduino_GFX *gfx, String str) {
  int16_t x = get_align_horizontal(gfx, str);
  int16_t y = get_align_vertical(gfx, str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text left-aligned with a margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_left(Arduino_GFX *gfx, String str, int16_t y) {
  gfx->setCursor(MARGIN, y);
  gfx->print(str);
}

/**
 * @brief Draw text right-aligned with a margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_right(Arduino_GFX *gfx, String str, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor(LCD_WIDTH - w - MARGIN, y);
  gfx->print(str);
}

/**
 * @brief Draw text aligned at the top with a margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_top(Arduino_GFX *gfx, String str, int16_t x) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor(x, h + MARGIN);
  gfx->print(str);
}

/**
 * @brief Draw text aligned at the bottom with a margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_bottom(Arduino_GFX *gfx, String str, int16_t x) {
  gfx->setCursor(x, LCD_HEIGHT - MARGIN);
  gfx->print(str);
}

/**
 * @brief Draw text top-centered with a top margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param margin Margin from the top (in pixels).
 */
void draw_align_top_centered(Arduino_GFX *gfx, String str, int16_t margin) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (LCD_WIDTH - w) / 2;
  int16_t y = margin - y1;  // align top of bounding box

  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text bottom-centered with a bottom margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param margin Margin from the bottom (in pixels).
 */
void draw_align_bottom_centered(Arduino_GFX *gfx, String str, int16_t margin) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (LCD_WIDTH - w) / 2;
  int16_t y = (LCD_HEIGHT - margin) - (y1 + h);  // align bottom

  gfx->setCursor(x, y);
  gfx->print(str);
}

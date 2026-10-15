/**
 * @file ui_theme.h
 * @brief Shared dark companion palette and bounded display primitives.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>

constexpr uint16_t UI_BG = 0x0000;
/**
 * @brief Design field of the menu, desk, and tool widgets.
 *
 * Widget geometry is fixed for the 240x280 reference panel, so larger panels
 * keep that field anchored at the top-left and taps outside it stay dead
 * instead of reaching a button that is not drawn there.
 */
constexpr int16_t UI_DESIGN_W = 240;
constexpr int16_t UI_DESIGN_H = 280;
extern uint16_t UI_CARD;
extern uint16_t UI_TEXT;
extern uint16_t UI_MUTED;
extern uint16_t UI_ACCENT;
extern uint16_t UI_WARM;

/** @brief Apply the eye color to text, icons, controls and tinted cards. @param color RGB565 eye color. */
void ui_theme_color(uint16_t color);

/** @brief Sanitize UTF-8 for the built-in font. @param text Source text. @return Displayable text. */
String ui_ascii(const String &text);
/**
 * @brief Draw clipped text, never spilling into an adjacent region.
 * @param gfx Canvas.
 * @param text UTF-8 text.
 * @param x Left edge.
 * @param y Top edge.
 * @param width Available width.
 * @param size Font scale.
 * @param color Foreground.
 */
void ui_label(Arduino_GFX *gfx, const String &text, int16_t x, int16_t y,
	int16_t width, uint8_t size = 1, uint16_t color = UI_TEXT);
/** @brief Draw a rounded card. @param gfx Canvas. @param y Top. @param height Height. */
void ui_card(Arduino_GFX *gfx, int16_t y, int16_t height);
/**
 * @brief Draw the shared header and screen position dots.
 * @param gfx Canvas.
 * @param title Screen name.
 * @param connected Bluetooth flag.
 * @param page Selected screen index.
 */
void ui_chrome(Arduino_GFX *gfx, const char *title, bool connected, uint8_t page);
/**
 * @brief Draw a compact speaker glyph showing whether audio can sound.
 * @param gfx Canvas.
 * @param x Left edge; glyph spans x..x+21 and y..y+12.
 * @param y Top edge.
 */
void ui_sound_badge(Arduino_GFX *gfx, int16_t x, int16_t y);
/** @brief Erase the sound badge rectangle. @param gfx Canvas. @param x Left edge. @param y Top edge. */
void ui_sound_badge_clear(Arduino_GFX *gfx, int16_t x, int16_t y);
/**
 * @brief Draw a calm empty state.
 * @param gfx Canvas.
 * @param title Primary message.
 * @param hint Next action.
 */
void ui_empty(Arduino_GFX *gfx, const char *title, const char *hint);
/**
 * @brief Draw a vector weather symbol with no external bitmap assets.
 * @param gfx Canvas.
 * @param code Chronos condition code.
 * @param x Center X.
 * @param y Center Y.
 */
void ui_weather_icon(Arduino_GFX *gfx, int code, int16_t x, int16_t y);

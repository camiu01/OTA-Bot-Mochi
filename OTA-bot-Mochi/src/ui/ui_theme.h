/**
 * @file ui_theme.h
 * @brief Shared dark companion palette and bounded display primitives.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>

constexpr uint16_t UI_BG = 0x0000;
/**
 * @brief Design space of the menu, desk, and tool widgets.
 *
 * Every widget is laid out in this 240x280 space and then scaled to the
 * actual panel with ui_s(), so larger panels fill the screen instead of
 * framing a small island with black margins. The 240x280 reference panel
 * scales 1:1 and keeps the original pixel geometry.
 */
constexpr int16_t UI_DESIGN_W = 240;
constexpr int16_t UI_DESIGN_H = 280;
extern uint16_t UI_CARD;
extern uint16_t UI_TEXT;
extern uint16_t UI_MUTED;
extern uint16_t UI_ACCENT;
extern uint16_t UI_WARM;

/** @brief Layout scale of the design space on this panel (percent, 100 on the reference). @return Scale percent, clamped 100..200. */
int16_t ui_scale();
/**
 * @brief Scale one design-space length to the panel.
 * @param value Length in 240x280 design pixels.
 * @return Panel pixels.
 */
inline int16_t ui_s(int16_t value) { return int16_t(int32_t(value) * ui_scale() / 100); }
/**
 * @brief Text size for the current panel.
 *
 * Small labels and values grow one step on scaled-up panels so they do not
 * drown inside the larger widgets; display sizes (clock, timer, distance)
 * stay as tuned.
 * @param size Design text size.
 * @return Effective text size.
 */
inline uint8_t ui_text_size(uint8_t size) { return (size <= 3 && ui_scale() >= 140) ? uint8_t(size + 1) : size; }

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

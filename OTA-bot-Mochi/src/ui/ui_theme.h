/**
 * @file ui_theme.h
 * @brief Shared dark companion palette and bounded display primitives.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>

constexpr uint16_t UI_BG = 0x0000;
/** @brief Blend RGB565 colors with opacity in the range 0–256. */
inline uint16_t ui_mix(uint16_t base, uint16_t ink, uint16_t opacity) {
	const uint32_t keep = 256 - opacity;
	const uint32_t r = ((base >> 11) * keep + (ink >> 11) * opacity + 128) >> 8;
	const uint32_t g = (((base >> 5) & 63) * keep + ((ink >> 5) & 63) * opacity + 128) >> 8;
	const uint32_t b = ((base & 31) * keep + (ink & 31) * opacity + 128) >> 8;
	return uint16_t((r << 11) | (g << 5) | b);
}
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
extern uint16_t UI_LINE;	/**< Hairline outline of cards and secondary buttons. */
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
 * @brief Rounded surface in panel pixels; the radius never exceeds half the height.
 * @param gfx Canvas. @param x Left. @param y Top. @param w Width. @param h Height.
 * @param fill Surface color. @param outline Draw the UI_LINE hairline.
 */
void ui_box(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t fill, bool outline = false);
/**
 * @brief Button in panel pixels with a centered label: accent fill when primary, outlined card otherwise.
 * @param gfx Canvas. @param x Left. @param y Top. @param w Width. @param h Height.
 * @param label Text. @param primary Accent style. @param enabled Muted when false.
 */
void ui_button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, const String &label,
	bool primary = false, bool enabled = true);
/**
 * @brief Single line centered on the panel, clipped to the card's inner width.
 * @param gfx Canvas. @param text Text. @param y Top (design). @param size Font scale. @param color Ink.
 */
void ui_center(Arduino_GFX *gfx, const String &text, int16_t y, uint8_t size, uint16_t color);
/**
 * @brief Rounded progress track with a filled part.
 * @param gfx Canvas. @param x Left. @param y Top. @param w Width. @param h Height (panel pixels).
 * @param permille Filled share, 0..1000 (clamped). @param color Fill.
 */
void ui_bar(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int32_t permille, uint16_t color);
/**
 * @brief Status line with a leading dot: accent when good, muted otherwise; the text follows the dot color's weight.
 * @param gfx Canvas. @param y Top (design). @param good State is healthy. @param text Label.
 */
void ui_status(Arduino_GFX *gfx, int16_t y, bool good, const String &text);
/**
 * @brief Draw the shared header and screen position dots.
 * @param gfx Canvas.
 * @param title Screen name.
 * @param connected Bluetooth flag.
 * @param page Selected screen index.
 */
void ui_chrome(Arduino_GFX *gfx, const char *title, bool connected, uint8_t page, bool settings_page = false);
/**
 * @brief Draw a compact speaker glyph showing whether audio can sound.
 * @param gfx Canvas.
 * @param x Left edge; glyph spans x..x+21 and y..y+12.
 * @param y Top edge.
 */
void ui_sound_badge(Arduino_GFX *gfx, int16_t x, int16_t y);
/**
 * @brief Draw a battery outline and inset bolt while the PMU reports charging.
 * @param gfx Canvas.
 * @param x Left edge; glyph spans x..x+7 and y..y+12.
 * @param y Top edge.
 */
void ui_charge_badge(Arduino_GFX *gfx, int16_t x, int16_t y);
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

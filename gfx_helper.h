#ifndef GFX_HELPER_H
#define GFX_HELPER_H

#include <Arduino_GFX_Library.h>
#include "pin_config.h"

/**
 * @brief Default background color for the screen.
 */
#define BACKGROUND BLACK

/**
 * @brief Default text color.
 */
#define TEXT_COLOR WHITE

/**
 * @brief Default color for graphic elements.
 */
#define ELEMENT_COLOR WHITE

/**
 * @brief Default margin (in pixels) used for alignment.
 */
#define MARGIN 10

/**
 * @brief Top margin (in pixels) used for header alignment.
 */
#define HEADER_MARGIN_TOP 15

/**
 * @brief Vertical spacer from the top (in pixels).
 */
#define TOP_SPACER_Y 55

/**
 * @brief Calculate X position to horizontally center text.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @return X coordinate for centered placement.
 */
int16_t get_align_horizontal(Arduino_GFX *gfx, String str);

/**
 * @brief Calculate Y position to vertically center text.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @return Y coordinate (baseline) for centered placement.
 */
int16_t get_align_vertical(Arduino_GFX *gfx, String str);

/**
 * @brief Draw text horizontally centered.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_horizontal(Arduino_GFX *gfx, String str, int16_t y);

/**
 * @brief Draw text vertically centered.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_vertical(Arduino_GFX *gfx, String str, int16_t x);

/**
 * @brief Draw text centered both horizontally and vertically.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 */
void draw_align_center(Arduino_GFX *gfx, String str);

/**
 * @brief Draw text left-aligned at given Y coordinate.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_left(Arduino_GFX *gfx, String str, int16_t y);

/**
 * @brief Draw text right-aligned at given Y coordinate.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_right(Arduino_GFX *gfx, String str, int16_t y);

/**
 * @brief Draw text aligned at the top with given X coordinate.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_top(Arduino_GFX *gfx, String str, int16_t x);

/**
 * @brief Draw text aligned at the bottom with given X coordinate.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_bottom(Arduino_GFX *gfx, String str, int16_t x);

/**
 * @brief Draw text top-centered with a top margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param margin Margin from the top (in pixels).
 */
void draw_align_top_centered(Arduino_GFX *gfx, String str, int16_t margin);

/**
 * @brief Draw text bottom-centered with a bottom margin.
 * @param gfx Pointer to the display object.
 * @param str Text string.
 * @param margin Margin from the bottom (in pixels).
 */
void draw_align_bottom_centered(Arduino_GFX *gfx, String str, int16_t margin);

#endif // GFX_HELPER_H

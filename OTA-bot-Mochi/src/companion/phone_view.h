/** @file phone_view.h @brief Contextual Android-only phone tools; the carousel retains seven destinations. */
#pragma once
#include <Arduino_GFX_Library.h>
/** @brief Render one phone tool from owned snapshots. @param gfx Canvas. @param day Local day of year. */
void phone_view_draw(Arduino_GFX *gfx, int day);
/** @brief Handle released phone-tool taps on the main task. @param x Panel X. @param y Panel Y. */
void phone_view_tap(int16_t x, int16_t y);
/** @brief Handle vertical gestures inside subviews. @param step Direction. @return Consumed. */
bool phone_view_scroll(int8_t step);

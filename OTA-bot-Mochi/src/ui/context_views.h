/**
 * @file context_views.h
 * @brief Contextual desk tools and large navigation, outside the primary carousel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "screens.h"
/** @brief Draw a desk tool. @param gfx Canvas. @param watch Clock. @param page Tool. @param data Snapshot. @param now Time. */
void context_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, const UiSnapshot &data, uint32_t now);
/** @brief Draw a large active route. @param gfx Canvas. @param data Snapshot. */
void context_route(Arduino_GFX *gfx, const UiSnapshot &data);
/** @brief Draw the bottom action row in the Settings overflow style. @param gfx Canvas. @param label Text. @param primary Accent fill for the page's main action. */
void context_footer(Arduino_GFX *gfx, const char *label, bool primary = false);

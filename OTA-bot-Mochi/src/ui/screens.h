/**
 * @file screens.h
 * @brief Companion utility screens and incoming-call overlay.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_state.h"
#include <Arduino_GFX_Library.h>

enum class UiPage : uint8_t {
	Eyes, Clock, Navigation, Notification, Weather, Settings,
	Desk, BigClock, Overview, Timer, Device, Pong, AnalogClock, Count
};

/** @brief Find the primary carousel parent. @param page Page. @return One of six main pages. */
inline UiPage screens_parent(UiPage page) {
	return uint8_t(page) >= uint8_t(UiPage::Desk) ? UiPage::Clock : page;
}
/** @brief Swipe the six-page carousel, not all contextual tools. @param page Page. @param step Direction. @return Page. */
inline UiPage screens_swipe(UiPage page, int8_t step) {
	if (!step) { return page; }
	return static_cast<UiPage>((int(screens_parent(page)) + (step > 0 ? 1 : -1) + 6) % 6);
}
/** @brief Dispatch short parked-use utility taps. @param page Page. @param x X. @param y Y. @param now Time. @return Target. */
UiPage screens_tap(UiPage page, int16_t x, int16_t y, uint32_t now);

/** @brief Dispatch vertical gestures through the contextual layers. @param page Page. @param scroll +1 up, -1 down. @return Target. */
UiPage screens_scroll(UiPage page, int8_t scroll);

/** @brief Force utility screen redraw on entry. */
void screens_invalidate();
/**
 * @brief Render changed utility content, respecting the dashboard call policy.
 * @param gfx Canvas.
 * @param watch Chronos clock.
 * @param page Selected page.
 * @param now Millisecond timestamp.
 * @return True when the canvas changed.
 */
bool screens_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, uint32_t now);
/** @brief Check for an explicitly displayed call view. @return Visible call overlay. */
bool screens_calling();
/** @brief Keep routes and incoming calls readable. @return Whether idle dimming is inhibited. */
bool screens_keep_awake();

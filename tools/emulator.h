/**
 * @file emulator.h
 * @brief Shared state and entry points for the host SDL2 LCD emulator.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "Arduino_GFX_Library.h"
#include "ChronosESP32.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include "../OTA-bot-Mochi/src/ui/auto_page.h"
#include "../OTA-bot-Mochi/src/ui/screens.h"
#include "../OTA-bot-Mochi/src/touch/swipe.h"
#include "../OTA-bot-Mochi/src/touch/touch_actions.h"
#include <string>

constexpr int LCD_W = LCD_WIDTH;
constexpr int LCD_H = LCD_HEIGHT;
constexpr int SCALE = 3;
constexpr uint32_t FRAME_MS = 33;	/**< Same frame budget as the firmware loop. */
constexpr uint32_t SAMPLE_MS = 10;	/**< Same contact cadence as touch_poll(). */

extern Arduino_GFX gfx;
extern ChronosESP32 watch;
extern UiPage page;
extern SwipeTracker swipe;
extern TouchActions actions;
extern int8_t pending_step;
extern bool finger;
extern int16_t finger_x, finger_y;
extern uint32_t last_sample, last_frame;
extern int8_t flick_dir;
extern uint8_t flick_left;
extern std::string shot_path;

/** @brief Human-readable page title for the window bar. @param value Page. @return Title. */
const char *page_name(UiPage value);

/** @brief Resolve a lower-case --page value. @param name Page key. @param out Destination. @return True when known. */
bool page_from_name(const std::string &name, UiPage &out);

/** @brief Switch pages exactly like the firmware helper: clear and repaint. @param next Target. @param now Millisecond timestamp. */
void switch_screen(UiPage next, uint32_t now);

/** @brief Toggle master sound from the status strip; RAM-only until SAVE. @param now Millisecond timestamp. */
void toggle_quick_sound(uint32_t now);

/** @brief Push a fresh notification so the auto page policy jumps to Alerts. */
void inject_notification();

/** @brief Start or stop a navigation session through the normal Chronos path. */
void toggle_route();

/** @brief Write the current canvas as a portable pixmap. @param path Output file. @return Success. */
bool write_ppm(const std::string &path);

/** @brief Feed one synthetic contact sample through the production gesture policies. @param now Millisecond timestamp. */
void sample_touch(uint32_t now);

/** @brief Route taps, holds, petting, scrolls, and swipes like the firmware loop. @param now Millisecond timestamp. */
void handle_touch(uint32_t now);

/** @brief Translate window, wheel, key, and console input into firmware actions. @param event Event. @param renderer Renderer. @param running Loop flag. */
void handle_event(const SDL_Event &event, SDL_Renderer *renderer, bool &running);

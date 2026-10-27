/**
 * @file power_button_shutdown.h
 * @brief Main-task LCD shutdown integration; AMOLED uses its autonomous PMU.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../../pin_config.h"
#include "power_button.h"
#include "backlight.h"
#include "../sound/buzzer_output.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <ChronosESP32.h>

/** @brief On LCD PWR release, mute and blank, drop battery hold, then reboot on USB wake. */
inline void handle_power_button(ChronosESP32 &watch, Arduino_GFX *display, bool ready, uint32_t now) {
#if MOCHI_BOARD_LCD_169
	if (!power_button_poll(now)) { return; }
	if (!power_button_prepare_sleep()) { Serial.println("PWR wake configuration failed"); return; }
	buzzer_end();
	backlight_suspend();
	if (ready) { display->displayOff(); }
	watch.stop();
	if (!power_button_sleep()) { Serial.println("PWR sleep failed; restarting"); }
	ESP.restart();
#else
	(void)watch; (void)display; (void)ready; (void)now;
#endif
}

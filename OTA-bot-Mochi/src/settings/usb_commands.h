/**
 * @file usb_commands.h
 * @brief Bounded USB settings commands; no compilation or device upload commands.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "settings_model.h"

enum class UsbAction { Invalid, Help, Show, Set, Save, Defaults, RouteView, WeatherStatus, WeatherSync, SoundStatus, SoundTest, MoodNames };
struct UsbCommand {
	UsbAction action = UsbAction::Invalid;
	CompanionSettings profile;
	int8_t route_mode = -1;
};

/**
 * @brief Parse a full settings command without accepting extra tokens.
 * @param line Null-terminated input, at most 95 bytes.
 * @param current Active profile.
 * @return Parsed action and validated candidate profile.
 */
UsbCommand settings_parse(const char *line, const CompanionSettings &current);
/** @brief Consume a bounded amount of serial input on the main task. */
void settings_usb_poll();

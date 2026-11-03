/**
 * @file companion_policy.h
 * @brief Pure minute-accurate schedule policies with no hardware or persistence.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "companion_state.h"

/** @brief Evaluate a window including midnight wrap; identical endpoints mean all day. @param window Schedule. @param minute Local minute. @return Active. */
inline bool companion_window_active(const CompanionWindow &window, uint16_t minute) {
	if (!window.enabled || minute >= 1440) { return false; }
	if (window.start == window.end) { return true; }
	return window.start < window.end ? minute >= window.start && minute < window.end :
		minute >= window.start || minute < window.end;
}
/** @brief Match the library's Monday-first repeat mask, including explicit one-shot and daily values. @param alarm Alarm. @param hour Hour. @param minute Minute. @param weekday Sunday=0. @return Due. */
inline bool companion_alarm_due(const Alarm &alarm, uint8_t hour, uint8_t minute, uint8_t weekday) {
	if (!alarm.enabled || alarm.hour != hour || alarm.minute != minute || weekday > 6) { return false; }
	if (alarm.repeat == 128 || alarm.repeat == 127) { return true; }
	return (alarm.repeat & (1U << (weekday ? weekday - 1 : 6))) != 0;
}
/** @brief Match interval slots relative to the window start, including midnight. @param window Reminder. @param minute Local minute. @return Due. */
inline bool companion_reminder_due(const CompanionWindow &window, uint16_t minute) {
	return window.interval && companion_window_active(window, minute) &&
		uint16_t((minute + 1440 - window.start) % 1440) % window.interval == 0;
}

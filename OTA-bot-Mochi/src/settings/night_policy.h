/**
 * @file night_policy.h
 * @brief Local-hour night scheduling, including intervals crossing midnight.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Resolve manual override and a synchronized automatic interval. @param manual Force night. @param automatic Enable schedule. @param start Inclusive hour. @param end Exclusive hour. @param hour Current local hour. @param synchronized Trusted clock. @return Effective night mode. */
inline bool night_active(bool manual, bool automatic, uint8_t start, uint8_t end,
	uint8_t hour, bool synchronized) {
	if (manual) { return true; }
	if (!automatic || !synchronized || start >= 24 || end >= 24 || hour >= 24 || start == end) { return false; }
	return start < end ? hour >= start && hour < end : hour >= start || hour < end;
}

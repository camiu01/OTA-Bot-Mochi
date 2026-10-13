/**
 * @file battery.h
 * @brief Best-effort LiPo charge estimate with plausibility gating for unverified hardware.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Sentinel for readings the unverified divider cannot plausibly produce. */
constexpr uint8_t BATTERY_UNAVAILABLE = 0xFF;

/** @brief Map a battery voltage to a bounded percentage. @param mv Battery millivolts. @return 0..100, or BATTERY_UNAVAILABLE. */
uint8_t battery_percent_mv(uint16_t mv);

/** @brief Convert one raw ADC sample through the assumed divider and plausibility window. @param raw ADC reading. @return Battery millivolts, or 0 when implausible. */
uint16_t battery_mv_raw(uint16_t raw);

/** @brief Require consecutive consistent samples before trusting or doubting the pin. */
struct BatteryTracker {
	uint8_t plausible = 0;		/**< Consecutive plausible samples. */
	uint8_t implausible = 0;	/**< Consecutive implausible samples. */
	uint8_t percent = BATTERY_UNAVAILABLE;	/**< Latest trusted estimate. */
	/** @brief Fold one raw sample: three plausible adopt it, five implausible drop it. @param raw ADC reading. */
	void update(uint16_t raw);
};

/** @brief Read the battery pin on a slow cadence; safe when no divider is fitted. @param now Millisecond timestamp. */
void battery_sample(uint32_t now);
/** @brief Latest trusted estimate. @return Percentage 0..100, or BATTERY_UNAVAILABLE. */
uint8_t battery_percent();

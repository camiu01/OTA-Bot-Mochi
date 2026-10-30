/**
 * @file battery.cpp
 * @brief Conservative ADC/PMU sampling behind a plausibility gate; never blocks or persists.
 * @details
 * The battery divider on the Waveshare ESP32-S3-Touch-LCD-1.69 OLD revision is not
 * verified. Readings outside a plausible LiPo window are treated as absent hardware,
 * so the UI can say "not measured" instead of showing a wrong percentage. The ADC
 * scale is the approximate uncalibrated 12-bit full scale at ~11 dB attenuation.
 * The AMOLED V2 has no divider: its AXP2101 PMU reports the cell voltage in
 * millivolts, and the same tracker curve turns that into a percentage.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "battery.h"
#include "charging.h"
#include "../../pin_config.h"
#include <Arduino.h>

namespace {
constexpr uint32_t SAMPLE_INTERVAL_MS = 30000;
constexpr uint32_t PRIME_INTERVAL_MS = 2000;
constexpr uint8_t PRIME_SAMPLES = 3;
constexpr uint8_t ADOPT_SAMPLES = 3;
constexpr uint8_t DOUBT_SAMPLES = 5;
constexpr uint16_t ADC_FULL_SCALE_MV = 3100;
constexpr uint16_t DIVIDER = 2;
constexpr uint16_t PLAUSIBLE_LOW_MV = 2800;
constexpr uint16_t PLAUSIBLE_HIGH_MV = 4350;

/** @brief Piecewise LiPo discharge anchors, interpolated linearly. */
struct Anchor { uint16_t mv; uint8_t percent; };
const Anchor curve[] = {
	{3000, 0}, {3300, 5}, {3450, 10}, {3680, 20}, {3740, 30}, {3770, 40},
	{3820, 50}, {3870, 60}, {3920, 70}, {3980, 80}, {4060, 90}, {4200, 100}
};
constexpr uint8_t CURVE_POINTS = sizeof(curve) / sizeof(curve[0]);

BatteryTracker tracker;
uint32_t last_sample = 0;
uint8_t samples = 0;
}

uint8_t battery_percent_mv(uint16_t mv) {
	if (mv <= curve[0].mv) { return 0; }
	for (uint8_t i = 1; i < CURVE_POINTS; ++i) {
		if (mv <= curve[i].mv) {
			const Anchor &low = curve[i - 1], &high = curve[i];
			return low.percent +
				uint8_t(uint32_t(mv - low.mv) * (high.percent - low.percent) / (high.mv - low.mv));
		}
	}
	return 100;
}

uint16_t battery_mv_raw(uint16_t raw) {
	uint32_t pin_mv = uint32_t(raw) * ADC_FULL_SCALE_MV / 4095;
	uint32_t battery_mv = pin_mv * DIVIDER;
	if (battery_mv < PLAUSIBLE_LOW_MV || battery_mv > PLAUSIBLE_HIGH_MV) { return 0; }
	return uint16_t(battery_mv);
}

void BatteryTracker::update_mv(uint16_t mv) {
	if (mv >= PLAUSIBLE_LOW_MV && mv <= PLAUSIBLE_HIGH_MV) {
		implausible = 0;
		if (plausible < 255) { ++plausible; }
		if (plausible >= ADOPT_SAMPLES) { percent = battery_percent_mv(mv); }
		return;
	}
	plausible = 0;
	if (implausible < 255) { ++implausible; }
	if (implausible >= DOUBT_SAMPLES) { percent = BATTERY_UNAVAILABLE; }
}

void BatteryTracker::update(uint16_t raw) { update_mv(battery_mv_raw(raw)); }

void battery_sample(uint32_t now) {
	uint32_t interval = samples < PRIME_SAMPLES ? PRIME_INTERVAL_MS : SAMPLE_INTERVAL_MS;
	if (samples && uint32_t(now - last_sample) < interval) { return; }
	last_sample = now;
	if (samples < 255) { ++samples; }
#if BAT_ADC >= 0
	tracker.update(uint16_t(analogRead(BAT_ADC)));
#elif defined(MOCHI_BOARD_AMOLED_18)
	// The AMOLED V2 has no divider; the AXP2101 reports millivolts directly.
	uint16_t mv = 0;
	tracker.update_mv(axp2101_battery_mv(mv) ? mv : 0);
#else
	// No ADC divider and no PMU on this board; the tracker settles on "not measured".
	tracker.update(0);
#endif
}

uint8_t battery_percent() { return tracker.percent; }

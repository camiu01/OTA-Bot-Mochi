/**
 * @file charging.h
 * @brief Debounced, read-only charge-start events with no inferred LCD charging state.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Unknown readings never count as a charger disconnect. */
enum class ChargeState : uint8_t { Unknown, Idle, Charging };

/** @brief Emit one event per stable transition after establishing a silent boot baseline. */
class ChargingTracker {
public:
	/** @brief Fold two consistent samples; ignore missing hardware and read errors. @param state PMU state. @return A new charge start. */
	bool update(ChargeState state) {
		if (state == ChargeState::Unknown) { _samples = 0; return false; }
		if (state != _candidate) { _candidate = state; _samples = 0; }
		if (_samples < 2) { ++_samples; }
		if (_samples < 2 || state == _stable) { return false; }
		bool started = _stable != ChargeState::Unknown && state == ChargeState::Charging;
		_stable = state;
		return started;
	}
private:
	ChargeState _candidate = ChargeState::Unknown, _stable = ChargeState::Unknown;
	uint8_t _samples = 0;
};

/** @brief Sample the AMOLED PMU at 500 ms intervals on the existing shared I2C bus. @param now Milliseconds. @return New stable charge start; always false on LCD. */
bool charging_poll(uint32_t now);

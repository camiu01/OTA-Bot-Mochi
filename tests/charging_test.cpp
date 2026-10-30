/**
 * @file charging_test.cpp
 * @brief Charge-edge debounce, unknown readings, shared-bus status and counter rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/power/charging.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <cassert>
#include <iostream>

/** @brief Ignore boot-on-USB, noisy transitions and errors without replaying an event. */
void policy_checks() {
	ChargingTracker tracker;
	assert(!tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Unknown));
	assert(!tracker.update(ChargeState::Charging) && !tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Idle));
	assert(!tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Idle) && !tracker.update(ChargeState::Idle));
	assert(!tracker.update(ChargeState::Charging) && tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Charging));
	assert(!tracker.update(ChargeState::Unknown));
	assert(!tracker.update(ChargeState::Charging) && !tracker.update(ChargeState::Charging));
}

/** @brief Read only PMU identity/status, rejecting missing devices and reserved states. */
void hardware_checks() {
#if defined(MOCHI_BOARD_AMOLED_18)
	auto &bus = i2c_bus();
	assert(!charging_poll(0));
	bus.present[0x34] = true;
	auto &registers = bus.registers[0x34]; registers[0x03] = 0x4A;
	registers[0x01] = 0x40;
	assert(!charging_poll(500) && !charging_poll(1000));
	registers[0x01] = 0x20;
	assert(!charging_poll(1500) && !charging_poll(1501) && charging_poll(2000));
	bus.read_ok = false; assert(!charging_poll(2500)); bus.read_ok = true;
	assert(!charging_poll(3000) && !charging_poll(3500));
	registers[0x01] = 0x60;
	assert(!charging_poll(4000) && !charging_poll(4500));
	registers[0x01] = 0;
	assert(!charging_poll(5000) && !charging_poll(5500));
	registers[0x03] = 0; registers[0x01] = 0x20;
	assert(!charging_poll(6000) && !charging_poll(6500));
	registers[0x03] = 0x4A;
	assert(!charging_poll(UINT32_MAX - 200) && charging_poll(300));
	assert(registers[0x01] == 0x20 && registers[0x03] == 0x4A);
	// Battery voltage uses REG00[3] presence plus the 13-bit REG34/REG35 millivolt ADC.
	uint16_t mv = 0;
	registers[0x00] = 0x08; registers[0x34] = 0x10; registers[0x35] = 0x30;
	assert(axp2101_battery_mv(mv) && mv == 4144);
	registers[0x34] = 0x3F; registers[0x35] = 0xFF;
	assert(axp2101_battery_mv(mv) && mv == 0x1FFF);
	registers[0x00] = 0;
	assert(!axp2101_battery_mv(mv));
	registers[0x00] = 0x08;
	bus.read_ok = false;
	assert(!axp2101_battery_mv(mv));
	bus.read_ok = true;
	registers[0x03] = 0;
	assert(!axp2101_battery_mv(mv));
	registers[0x03] = 0x4A;
#else
	assert(!charging_poll(0) && !charging_poll(1000) && !charging_poll(UINT32_MAX));
	uint16_t mv = 0;
	assert(!axp2101_battery_mv(mv));
#endif
}

/** @brief Run the portable and per-board charge policy checks. @return Success. */
int main() {
	policy_checks(); hardware_checks();
	std::cout << "Charge start debounce, silent baseline and read-only PMU checks passed\n";
	return 0;
}

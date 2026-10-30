/**
 * @file charging.cpp
 * @brief Read AXP2101 identity/current direction without changing charger configuration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "charging.h"
#include "../../pin_config.h"
#if defined(MOCHI_BOARD_AMOLED_18)
#include "../bus/i2c_bus.h"

namespace {
/** @brief Read one PMU status byte on the main task. @param reg Register. @param value Output. @return Complete response. */
bool read_status(uint8_t reg, uint8_t &value) {
	TwoWire &wire = i2c_bus();
	wire.beginTransmission(0x34); wire.write(reg);
	if (wire.endTransmission(false) != 0 || wire.requestFrom(uint8_t(0x34), uint8_t(1)) != 1) { return false; }
	value = uint8_t(wire.read());
	return true;
}
}
#endif

bool charging_poll(uint32_t now) {
#if defined(MOCHI_BOARD_AMOLED_18)
	static ChargingTracker tracker;
	static uint32_t sampled_at = 0;
	static bool sampled = false;
	if (sampled && uint32_t(now - sampled_at) < 500) { return false; }
	sampled = true; sampled_at = now;
	uint8_t identity = 0, status = 0;
	if (!i2c_bus_begin(IIC_SDA, IIC_SCL, 400000) || !read_status(0x03, identity) ||
		identity != 0x4A || !read_status(0x01, status)) { return tracker.update(ChargeState::Unknown); }
	// AXP2101 REG01[6:5] reports battery current direction; USB power alone is not charging.
	uint8_t direction = (status >> 5) & 3;
	ChargeState state = direction == 1 ? ChargeState::Charging : ChargeState::Idle;
	return tracker.update(direction == 3 ? ChargeState::Unknown : state);
#else
	(void)now;
	return false;
#endif
}

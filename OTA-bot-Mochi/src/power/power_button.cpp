/**
 * @file power_button.cpp
 * @brief Configure the PMU's autonomous power key on the existing shared bus.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "power_button.h"
#include "../../pin_config.h"
#include "../bus/i2c_bus.h"

#if defined(MOCHI_BOARD_AMOLED_18)
namespace {
constexpr uint8_t PMU_ADDRESS = 0x34;
constexpr uint8_t POWER_OFF_CONTROL = 0x22;
constexpr uint8_t POWER_KEY_TIMING = 0x27;

/** @brief Read one PMU register, rejecting missing or truncated responses. */
bool read_register(uint8_t reg, uint8_t &value) {
	TwoWire &wire = i2c_bus();
	wire.beginTransmission(PMU_ADDRESS);
	wire.write(reg);
	if (wire.endTransmission(false) != 0) { return false; }
	if (wire.requestFrom(PMU_ADDRESS, uint8_t(1)) != 1) { return false; }
	value = wire.read();
	return true;
}

/** @brief Write and verify a configuration byte before reporting success. */
bool write_register(uint8_t reg, uint8_t value) {
	TwoWire &wire = i2c_bus();
	wire.beginTransmission(PMU_ADDRESS);
	wire.write(reg);
	wire.write(value);
	if (wire.endTransmission() != 0) { return false; }
	uint8_t actual = 0;
	return read_register(reg, actual) && actual == value;
}
} // namespace
#endif

#if defined(MOCHI_BOARD_AMOLED_18)
bool power_button_init() {
	if (!i2c_bus_begin(IIC_SDA, IIC_SCL, 400000)) { return false; }
	uint8_t control = 0, timing = 0;
	if (!read_register(POWER_OFF_CONTROL, control) || !read_register(POWER_KEY_TIMING, timing)) { return false; }
	// AXP2101 datasheet sections 6.13.2.20/25: OFFLEVEL=4s, ONLEVEL=1s.
	uint8_t requested_timing = uint8_t((timing & 0xF0) | 0x02);
	if (timing != requested_timing && !write_register(POWER_KEY_TIMING, requested_timing)) { return false; }
	// Enable long-press power-off, not restart; preserve thermal protection.
	uint8_t requested_control = uint8_t((control & 0xFE) | 0x02);
	if (control != requested_control && !write_register(POWER_OFF_CONTROL, requested_control)) { return false; }
	return true;
}

bool power_button_poll(uint32_t) { return false; }
bool power_button_prepare_sleep() { return false; }
bool power_button_sleep() { return false; }
#endif

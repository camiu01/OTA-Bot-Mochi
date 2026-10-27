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
constexpr uint8_t KEY_IRQ_ENABLE = 0x41, KEY_IRQ_STATUS = 0x49;
constexpr uint8_t KEY_RELEASE = 0x01, KEY_PRESS = 0x02, KEY_LONG = 0x04;
constexpr uint8_t KEY_EVENTS = KEY_RELEASE | KEY_PRESS | KEY_LONG;
bool preview_ready = false, key_down = false, preview = false;
uint32_t last_poll = 0;

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

/** @brief Write a byte, including write-one-to-clear IRQ acknowledgements. */
bool send_register(uint8_t reg, uint8_t value) {
	TwoWire &wire = i2c_bus();
	wire.beginTransmission(PMU_ADDRESS);
	wire.write(reg);
	wire.write(value);
	return wire.endTransmission() == 0;
}

/** @brief Write and verify a configuration byte before reporting success. */
bool write_register(uint8_t reg, uint8_t value) {
	if (!send_register(reg, value)) { return false; }
	uint8_t actual = 0;
	return read_register(reg, actual) && actual == value;
}
} // namespace
#endif

#if defined(MOCHI_BOARD_AMOLED_18)
bool power_button_init() {
	preview_ready = key_down = preview = false;
	last_poll = 0;
	if (!i2c_bus_begin(IIC_SDA, IIC_SCL, 400000)) { return false; }
	uint8_t control = 0, timing = 0;
	if (!read_register(POWER_OFF_CONTROL, control) || !read_register(POWER_KEY_TIMING, timing)) { return false; }
	// AXP2101 datasheet sections 6.13.2.20/25: OFFLEVEL=4s, ONLEVEL=1s.
	// IRQLEVEL=1s leaves time for the farewell before autonomous power-off.
	static_assert(PowerButtonTiming::LONG_IRQ_MS == 1000 && PowerButtonTiming::POWER_OFF_MS == 4000,
		"Update AXP2101 timing bits when changing the shared shutdown timeline");
	uint8_t requested_timing = uint8_t((timing & 0xC0) | 0x02);
	if (timing != requested_timing && !write_register(POWER_KEY_TIMING, requested_timing)) { return false; }
	// Enable long-press power-off, not restart; preserve thermal protection.
	uint8_t requested_control = uint8_t((control & 0xFE) | 0x02);
	if (control != requested_control && !write_register(POWER_OFF_CONTROL, requested_control)) { return false; }
	uint8_t enabled = 0;
	if (!read_register(KEY_IRQ_ENABLE, enabled)) { return false; }
	if (!write_register(KEY_IRQ_ENABLE, uint8_t(enabled | KEY_EVENTS))) { return false; }
	// Ignore latched boot presses without clearing unrelated battery/USB events.
	preview_ready = send_register(KEY_IRQ_STATUS, KEY_EVENTS);
	return preview_ready;
}

bool power_button_preview(uint32_t now) {
	if (!preview_ready || uint32_t(now - last_poll) < 40) { return preview; }
	last_poll = now;
	uint8_t events = 0;
	if (!read_register(KEY_IRQ_STATUS, events)) { key_down = preview = false; return false; }
	events &= KEY_EVENTS;
	if (!events) { return preview; }
	if (!send_register(KEY_IRQ_STATUS, events)) { key_down = preview = false; return false; }
	if (events & KEY_PRESS) { key_down = true; }
	if (key_down && (events & KEY_LONG)) { preview = true; }
	// Release wins if both edges accumulated while the main task was busy.
	if (events & KEY_RELEASE) { key_down = preview = false; }
	return preview;
}

bool power_button_poll(uint32_t) { return false; }
bool power_button_prepare_sleep() { return false; }
bool power_button_sleep() { return false; }
#endif

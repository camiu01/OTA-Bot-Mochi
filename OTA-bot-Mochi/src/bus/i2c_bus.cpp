/**
 * @file i2c_bus.cpp
 * @brief One I2C controller routed to the board's shared sensor pads.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "i2c_bus.h"

static TwoWire s_bus(1);
static bool s_started = false;

bool i2c_bus_begin(uint8_t sda, uint8_t scl, uint32_t hz) {
	if (s_started) { return true; }
	if (!s_bus.begin(sda, scl, hz)) { return false; }
	s_bus.setTimeOut(5);
	s_started = true;
	return true;
}

TwoWire &i2c_bus() { return s_bus; }

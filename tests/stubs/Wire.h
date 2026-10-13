/**
 * @file Wire.h
 * @brief Native register-level I2C fake for sensor initialization checks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <array>
#include <vector>
#include <stdint.h>

class TwoWire {
	uint8_t address = 0;
	uint8_t pointer = 0;
	std::vector<uint8_t> sent;
public:
	explicit TwoWire(uint8_t = 0) {}
	bool begin_ok = true;
	bool read_ok = true;
	int fail_write_register = -1;
	std::array<bool, 128> present{};
	std::array<std::array<uint8_t, 256>, 128> registers{};
	bool begin(int, int, uint32_t = 0) { return begin_ok; }
	void setTimeOut(int) {}
	void beginTransmission(uint8_t target) { address = target; sent.clear(); }
	void write(uint8_t data) { sent.push_back(data); }
	int endTransmission(bool = true) {
		if (!present[address] || sent.empty()) { return 1; }
		pointer = sent[0];
		if (sent.size() > 1 && pointer == fail_write_register) { return 1; }
		for (size_t i = 1; i < sent.size(); ++i) { registers[address][pointer++] = sent[i]; }
		return 0;
	}
	uint8_t requestFrom(uint8_t target, uint8_t count) {
		address = target;
		return read_ok && present[address] ? count : 0;
	}
	int read() { return registers[address][pointer++]; }
};
inline TwoWire Wire;

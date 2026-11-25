/**
 * @file elm327_test.cpp
 * @brief ELM327 prompt framing, PID formulas, init/poll order and ignition-off recovery.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/vehicle/elm327.h"
#include <assert.h>
#include <math.h>
#include <iostream>
#include <string>

/** @brief Run one exchange with a scripted reply. */
static ObdStatus step(ElmSession &s, VehicleData &d, const char *expect, const char *reply) {
	assert(std::string(s.command()) == expect);
	return s.reply(reply, d);
}

static void framing_checks() {
	ElmBuffer b;
	const char *part1 = "41 0D", *part2 = " 3C\r\r>";
	assert(!b.feed(reinterpret_cast<const uint8_t *>(part1), strlen(part1)));
	assert(b.feed(reinterpret_cast<const uint8_t *>(part2), strlen(part2)));
	assert(std::string(b.text) == "41 0D 3C  ");
	std::string flood(300, 'A');
	b.clear();
	assert(!b.feed(reinterpret_cast<const uint8_t *>(flood.data()), flood.size()) && b.length == sizeof(b.text) - 1);
}

static void decode_checks() {
	VehicleData d;
	assert(elm_decode(0x0C, "410C1AF8", d) && d.rpm == 1726 && (d.valid & OBD_RPM));
	assert(elm_decode(0x0D, "SEARCHING... 41 0D 3C", d) && d.speed == 60);
	assert(elm_decode(0x05, "41057B", d) && d.coolant == 83);
	assert(elm_decode(0x11, "4111FF", d) && d.throttle == 100);
	assert(elm_decode(0x2F, "412F80", d) && fabsf(d.fuel - 50.2f) < 0.1f);
	assert(!elm_decode(0x0C, "410C1A", d));	// RPM needs two data bytes.
	assert(!elm_decode(0x0D, "NO DATA", d) && !elm_decode(0x0D, "410C1AF8", d));
	assert(elm_volts("12.6V", d) && fabsf(d.volts - 12.6f) < 0.01f);
	assert(elm_decode(0x01, "41 01 00 07 E5 00", d) && !d.mil && d.dtc == 0 && (d.valid & OBD_MIL));
	assert(!elm_volts("?", d) && !elm_volts("0.2V", d));
}

static void session_checks() {
	ElmSession s; VehicleData d;
	assert(s.timeout_ms() == 3000);
	for (const char *cmd : {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0"}) {
		assert(step(s, d, cmd, "OK") == ObdStatus::Starting);
	}
	assert(s.timeout_ms() == 8000);
	assert(step(s, d, "0100", "SEARCHING...UNABLE TO CONNECT") == ObdStatus::NoEcu);
	assert(step(s, d, "ATRV", "12.4V") == ObdStatus::NoEcu && fabsf(d.volts - 12.4f) < 0.01f);
	assert(step(s, d, "0100", "4100BE3EB811") == ObdStatus::Live && s.started());
	assert(step(s, d, "010C", "410C0FA0") == ObdStatus::Live && d.rpm == 1000);
	assert(step(s, d, "010D", "410D32") == ObdStatus::Live && d.speed == 50);
	assert(step(s, d, "0105", "410550") == ObdStatus::Live && d.coolant == 40);
	step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
	assert(step(s, d, "0111", "NO DATA") == ObdStatus::Live);
	for (int round = 0; round < 2; ++round) {	// Throttle unsupported: dropped after three misses.
		step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
		step(s, d, "012F", "NO DATA"); step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
		step(s, d, "ATRV", "13.9V"); step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
		step(s, d, "0101", "41018300"); step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
		assert(d.mil && d.dtc == 3 && (d.valid & OBD_MIL));
		step(s, d, "0105", "410550"); step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
		step(s, d, "0111", "NO DATA");
	}
	step(s, d, "010C", "410C0FA0"); step(s, d, "010D", "410D32");
	assert(std::string(s.command()) != "0111");
	ElmSession off; VehicleData e;
	for (const char *cmd : {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0"}) { step(off, e, cmd, "OK"); }
	step(off, e, "0100", "4100BE3EB811");
	step(off, e, "010C", "410C0FA0");
	assert(off.reply("", e) == ObdStatus::Live);	// Timed-out speed.
	off.reply("", e);	// Slow PID miss does not count toward ignition off.
	assert(off.reply("STOPPED", e) == ObdStatus::Live);
	assert(off.reply("NO DATA", e) == ObdStatus::NoEcu && !(e.valid & (OBD_RPM | OBD_SPEED)));
	assert(std::string(off.command()) == "0100");
}

int main() {
	framing_checks();
	decode_checks();
	session_checks();
	std::cout << "elm327 tests passed\n";
	return 0;
}

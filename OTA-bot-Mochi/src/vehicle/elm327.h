/**
 * @file elm327.h
 * @brief Transport-free ELM327 prompt framing, command sequencing and OBD-II PID decoding.
 *
 * The session resets the adapter, disables echo/linefeeds/spaces/headers, lets
 * it auto-detect the protocol, probes 0100, then polls RPM and speed every round
 * plus one slower value (coolant, throttle, fuel, adapter voltage). Three
 * consecutive empty engine replies mean ignition off: the session returns to
 * probing while ATRV keeps reporting battery voltage.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "obd_state.h"
#include <ctype.h>
#include <stdlib.h>

/** @brief Accumulate notification fragments until the '>' prompt. */
struct ElmBuffer {
	char text[128] = {};
	uint8_t length = 0;
	/** @brief Append bytes, ignoring overflow. @param data Bytes. @param size Count. @return Prompt seen. */
	bool feed(const uint8_t *data, size_t size) {
		for (size_t i = 0; i < size; ++i) {
			char c = char(data[i]);
			if (c == '>') { return true; }
			if (c == '\r' || c == '\n') { c = ' '; }
			if (c >= 32 && c < 127 && size_t(length) + 1 < sizeof(text)) { text[length++] = c; text[length] = 0; }
		}
		return false;
	}
	void clear() { length = 0; text[0] = 0; }
};

/** @brief Decode one PID reply ("410D3C") into the snapshot. @param pid Mode-01 PID. @param reply Adapter text. @param d Target. @return Value decoded. */
inline bool elm_decode(uint8_t pid, const char *reply, VehicleData &d) {
	char packed[128]; uint8_t n = 0;
	for (const char *p = reply; *p && size_t(n) + 1 < sizeof(packed); ++p) {
		if (!isspace((unsigned char)*p)) { packed[n++] = char(toupper((unsigned char)*p)); }
	}
	packed[n] = 0;
	char key[5]; key[0] = '4'; key[1] = '1';
	static const char hex[] = "0123456789ABCDEF";
	key[2] = hex[pid >> 4]; key[3] = hex[pid & 15]; key[4] = 0;
	const char *at = strstr(packed, key);
	if (!at) { return false; }
	at += 4;
	uint8_t bytes[2] = {}; uint8_t count = 0;
	while (count < 2 && isxdigit((unsigned char)at[0]) && isxdigit((unsigned char)at[1])) {
		char pair[3] = {at[0], at[1], 0};
		bytes[count++] = uint8_t(strtoul(pair, nullptr, 16)); at += 2;
	}
	uint8_t need = pid == 0x0C ? 2 : 1;
	if (count < need) { return false; }
	float a = bytes[0], b = bytes[1];
	if (pid == 0x0C) { d.rpm = (256 * a + b) / 4; d.valid |= OBD_RPM; }
	else if (pid == 0x0D) { d.speed = a; d.valid |= OBD_SPEED; }
	else if (pid == 0x05) { d.coolant = a - 40; d.valid |= OBD_COOLANT; }
	else if (pid == 0x11) { d.throttle = a * 100 / 255; d.valid |= OBD_THROTTLE; }
	else if (pid == 0x2F) { d.fuel = a * 100 / 255; d.valid |= OBD_FUEL; }
	else { return false; }
	return true;
}

/** @brief Parse "12.6V" from ATRV. @param reply Text. @param d Target. @return Voltage plausible. */
inline bool elm_volts(const char *reply, VehicleData &d) {
	const char *p = reply;
	while (*p && !isdigit((unsigned char)*p)) { ++p; }
	float v = strtof(p, nullptr);
	if (!*p || v < 5 || v > 30) { return false; }
	d.volts = v; d.valid |= OBD_VOLTS;
	return true;
}

/** @brief Command order and reply handling; the transport writes command() + '\r' and returns the framed reply. */
class ElmSession {
	static constexpr uint8_t INIT_STEPS = 6, SLOW = 4;
	uint8_t step = 0, round = 0, slow = 0, empty = 0;
	bool volts_turn = false;	/**< While probing a sleeping ECU, alternate with ATRV. */
	uint8_t misses[SLOW] = {};
	/** @brief Slow PID for the current round, or 0 for ATRV. */
	uint8_t slow_pid() const { static const uint8_t pids[SLOW] = {0x05, 0x11, 0x2F, 0}; return pids[slow]; }
	void next_slow() { for (uint8_t i = 0; i < SLOW; ++i) { slow = uint8_t((slow + 1) % SLOW); if (misses[slow] < 3) { return; } } }

public:
	/** @brief Restart after a new connection. */
	void reset() { *this = ElmSession{}; }
	/** @brief Probing or polling. @return Initialization finished. */
	bool started() const { return step > INIT_STEPS; }
	/** @brief Current command without the carriage return. @return ASCII command. */
	const char *command() const {
		static const char *init[INIT_STEPS + 1] = {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0", "0100"};
		if (step == INIT_STEPS && volts_turn) { return "ATRV"; }
		if (step <= INIT_STEPS) { return init[step]; }
		if (round == 0) { return "010C"; }
		if (round == 1) { return "010D"; }
		static const char *slow_text[SLOW] = {"0105", "0111", "012F", "ATRV"};
		return slow_text[slow];
	}
	/** @brief Reply budget: reset and protocol search are slow. @return Milliseconds. */
	uint32_t timeout_ms() const { return step == 0 ? 3000 : (step == INIT_STEPS && !volts_turn ? 8000 : 1500); }
	/**
	 * @brief Consume the framed reply (empty on timeout) and advance.
	 * @param reply Adapter text before '>'. @param d Snapshot to update. @return Status after this reply.
	 */
	ObdStatus reply(const char *reply, VehicleData &d) {
		if (step < INIT_STEPS) { ++step; return ObdStatus::Starting; }
		if (step == INIT_STEPS) {
			if (volts_turn) { elm_volts(reply, d); volts_turn = false; return ObdStatus::NoEcu; }
			if (strstr(reply, "4100")) {
				step = INIT_STEPS + 1; round = 0; empty = 0;
				for (uint8_t &miss : misses) { miss = 0; }
				return ObdStatus::Live;
			}
			volts_turn = true;	// Retry the probe after one adapter-voltage reading.
			return ObdStatus::NoEcu;
		}
		bool ok = false;
		if (round < 2) {
			ok = elm_decode(round == 0 ? 0x0C : 0x0D, reply, d);
			empty = ok ? 0 : uint8_t(empty + 1);
		} else {
			uint8_t pid = slow_pid();
			ok = pid ? elm_decode(pid, reply, d) : elm_volts(reply, d);
			misses[slow] = ok ? 0 : uint8_t(misses[slow] < 3 ? misses[slow] + 1 : 3);
			next_slow();
		}
		round = uint8_t((round + 1) % 3);
		if (empty >= 3) { step = INIT_STEPS; empty = 0; d.valid &= uint8_t(~(OBD_RPM | OBD_SPEED)); return ObdStatus::NoEcu; }
		return ObdStatus::Live;
	}
};

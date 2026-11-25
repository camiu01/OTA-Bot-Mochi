/**
 * @file elm327.h
 * @brief Transport-free ELM327 prompt framing, command sequencing and OBD-II PID decoding.
 *
 * The session resets the adapter, disables echo/linefeeds/spaces/headers, lets
 * it auto-detect the protocol, probes 0100, then polls RPM and speed every round
 * plus one slower value in turn: the selected ObdValue PIDs (adapter voltage via
 * ATRV) and the check-engine lamp with the stored trouble-code count. A PID that
 * misses three times drops out until the next probe; codes are never read in
 * detail or cleared. Three
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

/** @brief Mode-01 PID of a selectable value; 0 for none and for the battery (read with ATRV). */
inline uint8_t elm_value_pid(uint8_t value) {
	static const uint8_t pids[OBD_VALUE_COUNT] = {
		0, 0x05, 0x11, 0x2F, 0, 0x0F, 0x04, 0x0B, 0x0E, 0x06, 0x07, 0x14, 0x42, 0x10, 0x46, 0x33, 0x1F
	};
	return value < OBD_VALUE_COUNT ? pids[value] : 0;
}
/** @brief Apply the SAE J1979 formula of a selectable value. @param v ObdValue. @param a First byte. @param b Second byte. @return Display units. */
inline float elm_formula(uint8_t v, float a, float b) {
	switch (v) {
	case OBD_V_COOLANT: case OBD_V_INTAKE: case OBD_V_AMBIENT: return a - 40;	// C
	case OBD_V_THROTTLE: case OBD_V_FUEL: case OBD_V_LOAD: return a * 100 / 255;	// %
	case OBD_V_TIMING: return a / 2 - 64;	// degrees before TDC
	case OBD_V_STFT: case OBD_V_LTFT: return a * 100 / 128 - 100;	// % trim
	case OBD_V_O2: return a / 200;	// V, bank 1 sensor 1
	case OBD_V_ECU: return (256 * a + b) / 1000;	// V
	case OBD_V_MAF: return (256 * a + b) / 100;	// g/s
	case OBD_V_RUNTIME: return (256 * a + b) / 60;	// minutes
	default: return a;	// MAP and barometric kPa
	}
}
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
	uint8_t value = OBD_V_NONE;
	for (uint8_t v = 1; v < OBD_VALUE_COUNT && !value; ++v) { if (elm_value_pid(v) == pid) { value = v; } }
	bool wide = pid == 0x0C || value == OBD_V_ECU || value == OBD_V_MAF || value == OBD_V_RUNTIME;
	if (count < (wide ? 2 : 1)) { return false; }
	float a = bytes[0], b = bytes[1];
	if (pid == 0x0C) { d.rpm = (256 * a + b) / 4; d.valid |= OBD_RPM; }
	else if (pid == 0x0D) { d.speed = a; d.valid |= OBD_SPEED; }
	else if (pid == 0x01) { d.mil = bytes[0] & 0x80; d.dtc = bytes[0] & 0x7F; d.valid |= OBD_MIL; }
	else if (value) { d.value[value] = elm_formula(value, a, b); d.valid |= obd_bit(value); }
	else { return false; }
	return true;
}

/** @brief Parse "12.6V" from ATRV. @param reply Text. @param d Target. @return Voltage plausible. */
inline bool elm_volts(const char *reply, VehicleData &d) {
	const char *p = reply;
	while (*p && !isdigit((unsigned char)*p)) { ++p; }
	float v = strtof(p, nullptr);
	if (!*p || v < 5 || v > 30) { return false; }
	d.value[OBD_V_BATTERY] = v; d.valid |= obd_bit(OBD_V_BATTERY);
	return true;
}

/** @brief Command order and reply handling; the transport writes command() + '\r' and returns the framed reply. */
class ElmSession {
	static constexpr uint8_t INIT_STEPS = 6;
	uint8_t step = 0, round = 0, slow = 1, empty = 0;	/**< slow: ObdValue in turn, 0 = check-engine status. */
	bool volts_turn = false;	/**< While probing a sleeping ECU, alternate with ATRV. */
	uint32_t wanted = 0x1E;	/**< Bit per ObdValue to poll; defaults to the four default slots. */
	uint8_t misses[OBD_VALUE_COUNT] = {};
	/** @brief Polled this cycle. @param v Slow slot (0 = status). @return Wanted and not dropped. */
	bool active(uint8_t v) const { return misses[v] < 3 && (v == 0 || (wanted & (uint32_t(1) << v))); }
	void next_slow() {
		for (uint8_t i = 0; i < OBD_VALUE_COUNT; ++i) { slow = uint8_t((slow + 1) % OBD_VALUE_COUNT); if (active(slow)) { return; } }
	}

public:
	/** @brief Restart after a new connection. */
	void reset() { uint32_t keep = wanted; *this = ElmSession{}; wanted = keep; }
	/** @brief Choose the selectable values to poll; takes effect on the next slow turn. @param values Bit per ObdValue. */
	void want(uint32_t values) { wanted = values; if (!active(slow)) { next_slow(); } }
	/** @brief Probing or polling. @return Initialization finished. */
	bool started() const { return step > INIT_STEPS; }
	/** @brief Current command without the carriage return. @return ASCII command. */
	const char *command() const {
		static const char *init[INIT_STEPS + 1] = {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0", "0100"};
		if (step == INIT_STEPS && volts_turn) { return "ATRV"; }
		if (step <= INIT_STEPS) { return init[step]; }
		if (round == 0) { return "010C"; }
		if (round == 1) { return "010D"; }
		if (slow == 0) { return "0101"; }
		if (slow == OBD_V_BATTERY) { return "ATRV"; }
		static char text[5];
		static const char hex[] = "0123456789ABCDEF";
		uint8_t pid = elm_value_pid(slow);
		text[0] = '0'; text[1] = '1'; text[2] = hex[pid >> 4]; text[3] = hex[pid & 15]; text[4] = 0;
		return text;
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
			ok = slow == OBD_V_BATTERY ? elm_volts(reply, d) : elm_decode(slow ? elm_value_pid(slow) : 0x01, reply, d);
			misses[slow] = ok ? 0 : uint8_t(misses[slow] < 3 ? misses[slow] + 1 : 3);
			next_slow();
		}
		round = uint8_t((round + 1) % 3);
		if (empty >= 3) { step = INIT_STEPS; empty = 0; d.valid &= ~uint32_t(OBD_RPM | OBD_SPEED); return ObdStatus::NoEcu; }
		return ObdStatus::Live;
	}
};

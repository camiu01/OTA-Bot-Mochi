/**
 * @file ap_identity.h
 * @brief Independent persistent AP identity, never derived from router credentials.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Preferences.h>
#include <esp_random.h>
#include <stdio.h>
#include <string.h>

namespace ap_identity {
/** @brief Load or create once; refuse an unreadable record rather than silently changing credentials. */
inline bool load(char *ssid, char *password) {
	static constexpr char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
	char record[22] = {};
	Preferences storage;
	if (!storage.begin("mochi-ap", false)) { return false; }
	size_t size = storage.getBytesLength("identity");
	bool valid = size == sizeof(record) && storage.getBytes("identity", record, sizeof(record)) == sizeof(record);
	if (!size) {
		uint8_t random[14]; esp_fill_random(random, sizeof(random));
		record[0] = 1;
		for (size_t i = 0; i < 6; ++i) { snprintf(record + 1 + i * 2, 3, "%02x", unsigned(random[i])); }
		for (size_t i = 0; i < 8; ++i) { record[13 + i] = alphabet[random[6 + i] & 31]; }
		valid = storage.putBytes("identity", record, sizeof(record)) == sizeof(record);
		memset(random, 0, sizeof(random));
	}
	storage.end();
	valid = valid && record[0] == 1 && record[21] == 0;
	for (size_t i = 1; i < 13; ++i) { valid = valid && record[i] && strchr("0123456789abcdef", record[i]); }
	for (size_t i = 13; i < 21; ++i) { valid = valid && record[i] && strchr(alphabet, record[i]); }
	if (valid) {
		snprintf(ssid, 33, "Mochi-%.12s", record + 1);
		memcpy(password, record + 13, 8); password[8] = 0;
	}
	memset(record, 0, sizeof(record));
	return valid;
}
}

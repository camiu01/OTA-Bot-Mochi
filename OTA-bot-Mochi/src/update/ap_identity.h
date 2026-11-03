/**
 * @file ap_identity.h
 * @brief Independent persistent AP identity, never derived from router credentials.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Preferences.h>
#include "session_random.h"
#include <stdio.h>
#include <string.h>

namespace ap_identity {
/** @brief Load a 60-bit password, upgrading valid legacy identities without changing their SSID. */
inline bool load(char *ssid, char *password) {
	static constexpr char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
	char record[26] = {};
	Preferences storage;
	if (!storage.begin("mochi-ap", false)) { return false; }
	size_t size = storage.getBytesLength("identity");
	bool valid = (size == 22 || size == sizeof(record)) && storage.getBytes("identity", record, size) == size;
	bool legacy = valid && size == 22 && record[0] == 1 && record[21] == 0;
	valid = valid && (legacy || (size == sizeof(record) && record[0] == 2 && record[25] == 0));
	for (size_t i = 1; i < 13; ++i) { valid = valid && record[i] && strchr("0123456789abcdef", record[i]); }
	for (size_t i = 13; i < (legacy ? 21u : 25u); ++i) { valid = valid && record[i] && strchr(alphabet, record[i]); }
	if (!size || (valid && legacy)) {
		uint8_t random[18];
		if (!session_random(random, sizeof(random))) { storage.end(); return false; }
		record[0] = 2;
		if (!size) {
			for (size_t i = 0; i < 6; ++i) { snprintf(record + 1 + i * 2, 3, "%02x", unsigned(random[i])); }
		}
		for (size_t i = 0; i < 12; ++i) { record[13 + i] = alphabet[random[6 + i] & 31]; }
		record[25] = 0;
		valid = storage.putBytes("identity", record, sizeof(record)) == sizeof(record);
		memset(random, 0, sizeof(random));
	}
	storage.end();
	if (valid) {
		snprintf(ssid, 33, "Mochi-%.12s", record + 1);
		memcpy(password, record + 13, 13);
	}
	memset(record, 0, sizeof(record));
	return valid;
}
}

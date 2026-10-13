/**
 * @file Preferences.h
 * @brief In-memory NVS fake, including unavailable/failed storage cases.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <vector>
#include <cstring>
#include <stdint.h>

inline std::vector<uint8_t> test_profile;
inline bool test_storage_ready = true;
inline bool test_save_fails = false;
inline unsigned int test_flash_writes = 0;

class Preferences {
public:
	bool begin(const char *, bool) { return test_storage_ready; }
	size_t getBytesLength(const char *) { return test_profile.size(); }
	size_t getBytes(const char *, void *out, size_t length) {
		if (length != test_profile.size()) { return 0; }
		memcpy(out, test_profile.data(), length);
		return length;
	}
	size_t putBytes(const char *, const void *bytes, size_t length) {
		if (test_save_fails) { return 0; }
		const uint8_t *data = static_cast<const uint8_t *>(bytes);
		test_profile.assign(data, data + length);
		++test_flash_writes;
		return length;
	}
};

/**
 * @file Preferences.h
 * @brief In-memory NVS fake, including unavailable/failed storage cases.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <vector>
#include <cstring>
#include <stdint.h>
#include <map>
#include <string>

inline std::vector<uint8_t> test_profile;
inline bool test_storage_ready = true;
inline bool test_save_fails = false;
inline unsigned int test_flash_writes = 0;
inline std::map<std::string, std::vector<uint8_t>> test_nvs;

class Preferences {
	std::string name;
	/** @brief Isolate namespaces and keys while preserving existing profile fixtures. */
	std::vector<uint8_t> &record(const char *key) {
		return name == "mochi" && strcmp(key, "profile") == 0 ? test_profile : test_nvs[name + "/" + key];
	}
public:
	bool begin(const char *space, bool) { name = space; return test_storage_ready; }
	void end() { name.clear(); }
	size_t getBytesLength(const char *key) { return record(key).size(); }
	size_t getBytes(const char *key, void *out, size_t length) {
		if (length != record(key).size()) { return 0; }
		memcpy(out, record(key).data(), length);
		return length;
	}
	size_t putBytes(const char *key, const void *bytes, size_t length) {
		if (test_save_fails) { return 0; }
		const uint8_t *data = static_cast<const uint8_t *>(bytes);
		record(key).assign(data, data + length);
		++test_flash_writes;
		return length;
	}
};

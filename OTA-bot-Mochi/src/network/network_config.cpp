/**
 * @file network_config.cpp
 * @brief Fixed-format NVS storage without altering the display profile or partition map.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "network_config.h"
#include <Preferences.h>
#include <string.h>

namespace {
NetworkConfig config;
bool exists = false;
uint32_t revision = 0;
constexpr size_t RECORD_SIZE = 239;
/** @brief Reject unterminated text, controls and malformed UTF-8 before persistence/JSON output. */
bool text(const char *s, size_t capacity) {
	if (!memchr(s, 0, capacity)) { return false; }
	const uint8_t *p = reinterpret_cast<const uint8_t *>(s);
	while (*p) {
		uint8_t lead = *p++;
		if (lead < 32 || lead == 127) { return false; }
		if (lead < 128) { continue; }
		int count = lead >= 0xc2 && lead <= 0xdf ? 1 :
			(lead >= 0xe0 && lead <= 0xef ? 2 : (lead >= 0xf0 && lead <= 0xf4 ? 3 : 0));
		if (!count) { return false; }
		uint32_t value = lead & (0x7f >> (count + 1));
		for (int i = 0; i < count; ++i) {
			if ((*p & 0xc0) != 0x80) { return false; }
			value = (value << 6) | (*p++ & 0x3f);
		}
		uint32_t minimum = count == 1 ? 0x80 : (count == 2 ? 0x800 : 0x10000);
		if (value < minimum || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) { return false; }
	}
	return true;
}
/** @brief Detect corrupt/torn records; this is not encryption or authentication. */
uint32_t checksum(const uint8_t *data, size_t size) {
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < size; ++i) { hash = (hash ^ data[i]) * 16777619u; }
	return hash;
}
/** @brief Encode fields explicitly, excluding C++ padding and compiler-dependent booleans. */
void encode(const NetworkConfig &v, uint8_t *out) {
	memset(out, 0, RECORD_SIZE);
	out[0] = 1; out[1] = v.enabled; out[2] = v.weather; out[3] = v.zone;
	memcpy(out + 4, v.ssid, strlen(v.ssid)); memcpy(out + 37, v.password, strlen(v.password));
	memcpy(out + 102, v.server, strlen(v.server)); memcpy(out + 166, v.city, strlen(v.city));
	memcpy(out + 231, &v.latitude, 4); memcpy(out + 235, &v.longitude, 4);
}
}

bool network_config_valid(const NetworkConfig &v) {
	if (v.zone > 1 || !text(v.ssid, sizeof(v.ssid)) || !text(v.password, sizeof(v.password)) ||
		!text(v.server, sizeof(v.server)) || !text(v.city, sizeof(v.city))) { return false; }
	size_t p = strlen(v.password), n = strlen(v.server);
	if ((v.enabled && !v.ssid[0]) || (!v.ssid[0] && p) || (p && p < 8) || p > 64 ||
		!n || v.server[0] == '.' || v.server[n - 1] == '.') { return false; }
	for (size_t i = 0; i < p && p == 64; ++i) {
		char c = v.password[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) { return false; }
	}
	for (const char *c = v.server; *c; ++c) {
		if (!((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') ||
			(*c >= '0' && *c <= '9') || *c == '.' || *c == '-')) { return false; }
	}
	return (!v.weather || (v.enabled && v.city[0])) &&
		v.latitude >= -90000000 && v.latitude <= 90000000 &&
		v.longitude >= -180000000 && v.longitude <= 180000000;
}
bool network_config_init() {
	config = NetworkConfig{}; exists = false; ++revision;
	Preferences storage;
	if (!storage.begin("mochi-net", true)) { return false; }
	uint8_t data[RECORD_SIZE + 4] = {};
	bool read = storage.getBytesLength("config") == sizeof(data) &&
		storage.getBytes("config", data, sizeof(data)) == sizeof(data);
	storage.end();
	uint32_t stored = 0; memcpy(&stored, data + RECORD_SIZE, 4);
	if (!read || data[0] != 1 || data[1] > 1 || data[2] > 1 || checksum(data, RECORD_SIZE) != stored) { return false; }
	NetworkConfig next; next.enabled = data[1]; next.weather = data[2]; next.zone = data[3];
	memcpy(next.ssid, data + 4, 33); memcpy(next.password, data + 37, 65);
	memcpy(next.server, data + 102, 64); memcpy(next.city, data + 166, 65);
	memcpy(&next.latitude, data + 231, 4); memcpy(&next.longitude, data + 235, 4);
	if (!network_config_valid(next)) { return false; }
	config = next; exists = true; return true;
}
bool network_config_save(const NetworkConfig &next) {
	if (!network_config_valid(next)) { return false; }
	uint8_t data[RECORD_SIZE + 4]; encode(next, data);
	uint32_t sum = checksum(data, RECORD_SIZE); memcpy(data + RECORD_SIZE, &sum, 4);
	Preferences storage;
	if (!storage.begin("mochi-net", false)) { return false; }
	bool saved = storage.putBytes("config", data, sizeof(data)) == sizeof(data);
	storage.end();
	if (saved) { config = next; exists = true; ++revision; }
	memset(data, 0, sizeof(data));
	return saved;
}
const NetworkConfig &network_config() { return config; }
bool network_config_exists() { return exists; }
uint32_t network_config_revision() { return revision; }

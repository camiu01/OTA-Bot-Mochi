/**
 * @file ancs_parser.cpp
 * @brief Validate little-endian ANCS packets before exposing phone text.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ancs_parser.h"

/** @brief Clear a partial response. */
void AncsParser::reset() {
	alert_ = AncsAlert{};
	bytes_.clear();
	pending_ = false;
	seen_ = 0;
}

/** @brief Begin a bounded attribute response for one notification. */
bool AncsParser::event(const uint8_t *data, size_t length) {
	if (!data || length != 8 || data[0] > 2) { return false; }
	uint32_t uid = uint32_t(data[4]) | uint32_t(data[5]) << 8 |
		uint32_t(data[6]) << 16 | uint32_t(data[7]) << 24;
	if (data[0] == 2) {
		if (uid == alert_.uid) { reset(); }
		return false;
	}
	reset();
	alert_.uid = uid;
	pending_ = true;
	return true;
}

/** @brief Parse complete TLVs, retaining only the requested fields. */
bool AncsParser::append(const uint8_t *data, size_t length) {
	if (!pending_ || (!data && length) || bytes_.size() + length > 1536) { reset(); return false; }
	if (!length) { return false; }
	bytes_.append(reinterpret_cast<const char *>(data), length);
	if (bytes_.size() < 5) { return false; }
	if (uint8_t(bytes_[0]) != 0) { reset(); return false; }
	uint32_t uid = uint8_t(bytes_[1]) | uint32_t(uint8_t(bytes_[2])) << 8 |
		uint32_t(uint8_t(bytes_[3])) << 16 | uint32_t(uint8_t(bytes_[4])) << 24;
	if (uid != alert_.uid) { reset(); return false; }
	size_t at = 5;
	while (at + 3 <= bytes_.size()) {
		uint8_t id = uint8_t(bytes_[at]);
		size_t size = uint8_t(bytes_[at + 1]) | size_t(uint8_t(bytes_[at + 2])) << 8;
		if (size > 1024) { reset(); return false; }
		if (at + 3 + size > bytes_.size()) { return false; }
		std::string value = bytes_.substr(at + 3, size);
		if (id == 0) { alert_.app = value.substr(0, 64); seen_ |= 1; }
		if (id == 1) { alert_.title = value.substr(0, 160); seen_ |= 2; }
		if (id == 3) { alert_.message = value.substr(0, 1024); seen_ |= 4; }
		if (id == 5) { alert_.date = value.substr(0, 16); seen_ |= 8; }
		at += 3 + size;
	}
	bytes_.erase(5, at - 5);
	if (seen_ != 15) { return false; }
	pending_ = false;
	return true;
}

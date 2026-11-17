/**
 * @file face_favorites.h
 * @brief Optional starred faces, independent of exclusions; new IDs are never starred implicitly.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include <stddef.h>
#include <string.h>

class FaceFavorites {
	uint8_t mask[(EXPRESSION_COUNT + 7) / 8] = {};
public:
	static constexpr size_t BYTES = (EXPRESSION_COUNT + 7) / 8, RECORD_BYTES = BYTES + 3;
	/** @brief Inspect one valid star without changing the allow-list. */
	bool contains(Expression face) const {
		return uint8_t(face) < EXPRESSION_COUNT && (mask[uint8_t(face) / 8] & (1U << (uint8_t(face) % 8)));
	}
	/** @brief Set or clear a star; an empty favorites list is valid. */
	bool set(Expression face, bool enabled) {
		if (uint8_t(face) >= EXPRESSION_COUNT) { return false; }
		uint8_t &byte = mask[uint8_t(face) / 8], bit = uint8_t(1U << (uint8_t(face) % 8));
		if (enabled) { byte |= bit; } else { byte &= uint8_t(~bit); }
		return true;
	}
	/** @brief Count starred faces, including stars on currently excluded faces. */
	uint8_t count() const {
		uint8_t total = 0;
		for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) { total += contains(Expression(id)) ? 1 : 0; }
		return total;
	}
	/** @brief Compare the semantic bitmap, not storage metadata. */
	bool operator==(const FaceFavorites &other) const { return memcmp(mask, other.mask, BYTES) == 0; }
	/** @brief Encode an independent version/count/bitmap/checksum record. */
	void encode(uint8_t *out) const {
		out[0] = 1; out[1] = EXPRESSION_COUNT; memcpy(out + 2, mask, BYTES);
		out[RECORD_BYTES - 1] = 0xA5;
		for (size_t i = 0; i < RECORD_BYTES - 1; ++i) { out[RECORD_BYTES - 1] ^= out[i]; }
	}
	/** @brief Preserve old stars, leave appended IDs unstarred, and reject corruption atomically. */
	bool decode(const uint8_t *data, size_t length) {
		if (!data || length < 4 || data[0] != 1 || !data[1] || data[1] > EXPRESSION_COUNT ||
			length != size_t((data[1] + 7) / 8 + 3)) { return false; }
		uint8_t checksum = 0xA5;
		for (size_t i = 0; i < length - 1; ++i) { checksum ^= data[i]; }
		if (checksum != data[length - 1] || (data[1] % 8 && (data[length - 2] >> (data[1] % 8)))) { return false; }
		FaceFavorites next; memcpy(next.mask, data + 2, length - 3);
		*this = next; return true;
	}
};

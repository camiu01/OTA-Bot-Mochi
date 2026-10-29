/**
 * @file face_filter.h
 * @brief Portable global allow-list, with bounded scans and versioned independent storage.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include <stddef.h>
#include <string.h>

class FaceFilter {
	uint8_t mask[(EXPRESSION_COUNT + 7) / 8] = {};
public:
	static constexpr size_t BYTES = (EXPRESSION_COUNT + 7) / 8;
	static constexpr size_t RECORD_BYTES = BYTES + 3;
	/** @brief New/missing records enable every known expression. */
	FaceFilter() {
		for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) { mask[i / 8] |= uint8_t(1U << (i % 8)); }
	}
	/** @brief Reject invalid IDs without indexing the bitmap. */
	bool allows(Expression value) const {
		return uint8_t(value) < EXPRESSION_COUNT && (mask[uint8_t(value) / 8] & (1U << (uint8_t(value) % 8)));
	}
	/** @brief Count allowed expressions, including neutral and dizzy. */
	uint8_t count() const {
		uint8_t n = 0;
		for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) { if (allows(Expression(i))) { ++n; } }
		return n;
	}
	/** @brief Toggle a valid ID, refusing to exclude the last visible expression. */
	bool set(Expression value, bool enabled) {
		if (uint8_t(value) >= EXPRESSION_COUNT || (!enabled && allows(value) && count() == 1)) { return false; }
		uint8_t &byte = mask[uint8_t(value) / 8], bit = uint8_t(1U << (uint8_t(value) % 8));
		if (enabled) { byte |= bit; } else { byte &= uint8_t(~bit); }
		return true;
	}
	/** @brief Keep an allowed choice, otherwise fall back to the first allowed ID. */
	Expression resolve(Expression value) const {
		if (allows(value)) { return value; }
		for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) { if (allows(Expression(i))) { return Expression(i); } }
		return NEUTRAL;
	}
	/** @brief Compare semantic masks, not storage metadata. */
	bool operator==(const FaceFilter &other) const { return memcmp(mask, other.mask, BYTES) == 0; }
	/** @brief Encode stable version/count/mask/checksum, independent of the 13-byte profile. */
	void encode(uint8_t *out) const {
		out[0] = 1; out[1] = EXPRESSION_COUNT;
		memcpy(out + 2, mask, BYTES);
		out[RECORD_BYTES - 1] = 0xA5;
		for (size_t i = 0; i < RECORD_BYTES - 1; ++i) { out[RECORD_BYTES - 1] ^= out[i]; }
	}
	/** @brief Validate before assignment; older shorter catalogs enable newly appended faces. */
	bool decode(const uint8_t *data, size_t length) {
		if (!data || length < 4 || data[0] != 1 || !data[1] || data[1] > EXPRESSION_COUNT ||
			length != size_t((data[1] + 7) / 8 + 3)) { return false; }
		uint8_t sum = 0xA5;
		for (size_t i = 0; i < length - 1; ++i) { sum ^= data[i]; }
		if (sum != data[length - 1] || (data[1] % 8 && (data[length - 2] >> (data[1] % 8)))) { return false; }
		bool any = false;
		for (size_t i = 2; i < length - 1; ++i) { any = any || data[i] != 0; }
		if (!any) { return false; }
		FaceFilter next;
		for (uint8_t i = 0; i < data[1]; ++i) {
			if (!(data[2 + i / 8] & (1U << (i % 8)))) { next.mask[i / 8] &= uint8_t(~(1U << (i % 8))); }
		}
		if (!next.count()) { return false; }
		*this = next; return true;
	}
	/** @brief Produce a fixed-width lowercase hexadecimal bitmap for the local browser. */
	void hex(char *out) const {
		static constexpr char digits[] = "0123456789abcdef";
		for (size_t i = 0; i < BYTES; ++i) { out[i * 2] = digits[mask[i] >> 4]; out[i * 2 + 1] = digits[mask[i] & 15]; }
		out[BYTES * 2] = '\0';
	}
	/** @brief Strictly parse a fixed-width mask, refusing reserved bits or an empty selection. */
	bool parse_hex(const char *text) {
		if (!text || strlen(text) != BYTES * 2) { return false; }
		FaceFilter next;
		for (size_t i = 0; i < BYTES * 2; ++i) {
			char c = text[i];
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) { return false; }
			uint8_t digit = uint8_t(c <= '9' ? c - '0' : c - 'a' + 10);
			if (i % 2) { next.mask[i / 2] |= digit; } else { next.mask[i / 2] = uint8_t(digit << 4); }
		}
		if (!next.count() || (EXPRESSION_COUNT % 8 && (next.mask[BYTES - 1] >> (EXPRESSION_COUNT % 8)))) { return false; }
		*this = next; return true;
	}
};

/** @brief Share one main-task-only allow-list with portable selection policies. */
inline FaceFilter &face_filter() { static FaceFilter value; return value; }

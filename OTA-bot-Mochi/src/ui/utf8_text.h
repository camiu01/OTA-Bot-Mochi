/**
 * @file utf8_text.h
 * @brief Byte-budgeted prefixes that retain complete, valid UTF-8 code points only.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stddef.h>
#include <stdint.h>

/** @brief Stop before incomplete or malformed UTF-8 without reading past the source.
 * @param text Source bytes. @param length Available bytes. @param limit Byte budget. @return Prefix bytes. */
inline size_t utf8_prefix(const char *text, size_t length, size_t limit) {
	if (!text) { return 0; }
	size_t end = length < limit ? length : limit;
	size_t at = 0;
	while (at < end) {
		uint8_t lead = uint8_t(text[at]);
		size_t size = lead < 0x80 ? 1 : (lead >= 0xc2 && lead <= 0xdf ? 2 :
			(lead >= 0xe0 && lead <= 0xef ? 3 : (lead >= 0xf0 && lead <= 0xf4 ? 4 : 0)));
		if (!size || size > end - at) { break; }
		uint32_t value = lead & (0x7f >> size);
		for (size_t i = 1; i < size; ++i) {
			uint8_t next = uint8_t(text[at + i]);
			if ((next & 0xc0) != 0x80) { return at; }
			value = (value << 6) | (next & 0x3f);
		}
		if ((size == 2 && value < 0x80) || (size == 3 && value < 0x800) ||
			(size == 4 && value < 0x10000) || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) { break; }
		at += size;
	}
	return at;
}

/**
 * @file ota_image.h
 * @brief Bounded streaming compatibility checks, not firmware authentication.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../../pin_config.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/** @brief Embedded board/layout identity, retained by the streaming checker. @return Marker. */
inline const char *ota_image_marker() {
#if MOCHI_BOARD_LCD_169
	return "MOCHI-OTA-v1:lcd-169:16m-v1:END";
#else
	return "MOCHI-OTA-v1:amoled-18-v2:16m-v1:END";
#endif
}

/** @brief Strict decimal upload size, bounded by the inactive slot. @param text Header. @param limit Slot bytes. @return Size, zero if invalid. */
inline size_t ota_image_size(const char *text, size_t limit) {
	if (!text || !*text) { return 0; }
	size_t value = 0;
	for (; *text; ++text) {
		if (*text < '0' || *text > '9' || size_t(*text - '0') > limit ||
			value > (limit - size_t(*text - '0')) / 10) { return 0; }
		value = value * 10 + size_t(*text - '0');
	}
	return value >= 288 ? value : 0;
}

/** @brief Inspect an ESP32-S3 app header and a board/layout marker across arbitrary chunks. */
struct OtaImageCheck {
	uint8_t header[36] = {};
	size_t bytes = 0;
	size_t matched = 0;
	bool board_found = false;

	/** @brief Consume one bounded chunk. @param data Bytes. @param count Byte count. */
	void feed(const uint8_t *data, size_t count) {
		const char *marker = ota_image_marker();
		const size_t length = strlen(marker);
		for (size_t i = 0; i < count; ++i) {
			if (bytes < sizeof(header)) { header[bytes] = data[i]; }
			++bytes;
			if (board_found) { continue; }
			matched = data[i] == uint8_t(marker[matched]) ? matched + 1 :
				(data[i] == uint8_t(marker[0]) ? 1 : 0);
			if (matched == length) { board_found = true; }
		}
	}

	/** @brief Reject bootloaders, full images, and non-S3 headers before writing. @return App header. */
	bool app_header() const {
		return bytes >= sizeof(header) && header[0] == 0xe9 &&
			header[12] == 9 && header[13] == 0 &&
			header[32] == 0x32 && header[33] == 0x54 &&
			header[34] == 0xcd && header[35] == 0xab;
	}
};

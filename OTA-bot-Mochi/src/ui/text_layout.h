/**
 * @file text_layout.h
 * @brief Bounded, host-testable wrapping for the display's built-in font.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

struct TextLines {
	char line[40][35] = {};
	uint8_t count = 0;
	bool clipped = false;
};

/**
 * @brief Wrap ASCII text at word boundaries, splitting oversized words.
 * @param text Null-terminated, sanitized input.
 * @param columns Maximum visible columns, from 1 to 34.
 * @return Bounded lines, including an ellipsis when the buffer is exhausted.
 */
inline TextLines wrap_text(const char *text, uint8_t columns) {
	TextLines result;
	if (!text || columns == 0 || columns > 34) {
		return result;
	}
	while (*text && result.count < 40) {
		while (*text == ' ' || *text == '\r') { ++text; }
		if (!*text) { break; }
		size_t length = 0;
		while (text[length] && text[length] != '\n' && length < columns) { ++length; }
		size_t used = length;
		if (length == columns && text[length] && text[length] != ' ' &&
			text[length] != '\n') {
			for (size_t i = length; i > 0; --i) {
				if (text[i - 1] == ' ') { used = i - 1; break; }
			}
			if (used == 0) { used = length; }
		}
		memcpy(result.line[result.count], text, used);
		result.line[result.count++][used] = '\0';
		text += used;
		if (*text == '\n') { ++text; }
	}
	if (*text && result.count == 40) {
		result.clipped = true;
		size_t length = strlen(result.line[39]);
		size_t dots = columns < 3 ? columns : 3;
		size_t start = length > dots ? length - dots : 0;
		memset(result.line[39] + start, '.', dots);
		result.line[39][start + dots] = '\0';
	}
	return result;
}

/**
 * @brief Decide whether weather has exceeded its supported freshness interval.
 * @param now Current millisecond timestamp.
 * @param received Timestamp of the most recent weather observation.
 * @return True after three hours, including across millis rollover.
 */
inline bool weather_stale(uint32_t now, uint32_t received) {
	return static_cast<uint32_t>(now - received) >= 3UL * 60 * 60 * 1000;
}

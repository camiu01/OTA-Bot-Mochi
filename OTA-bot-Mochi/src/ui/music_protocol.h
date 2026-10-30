/**
 * @file music_protocol.h
 * @brief Bounded Apple Media Service entity decoding shared with native tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_state.h"
#include <cmath>
#include <cstdio>
#include <string>

/** @brief Decode a complete AMS entity value. @param music State. @param data Packet. @param size Bytes. @return Recognized valid update. */
inline bool music_entity(UiMusic &music, const uint8_t *data, size_t size) {
	if (!data || size < 3 || (data[2] & ~1U)) { return false; }
	if (data[0] == 0 && data[1] == 2) {
		if (size < 4 || size > 19 || data[2] || memchr(data + 3, 0, size - 3)) { return false; }
		std::string value(reinterpret_cast<const char *>(data + 3), size - 3);
		char *end = nullptr;
		float volume = strtof(value.c_str(), &end);
		if (end == value.c_str() || *end || !std::isfinite(volume) || volume < 0 || volume > 1) { return false; }
		music.volume = uint8_t(std::lround(volume * 100));
		return true;
	}
	if (data[0] == 0 && data[1] == 1) {
		if (size < 8 || size > 99 || memchr(data + 3, 0, size - 3)) { return false; }
		std::string value(reinterpret_cast<const char *>(data + 3), size - 3);
		int state = -1;
		float rate = 0, elapsed = 0;
		char trailing = 0;
		if (sscanf(value.c_str(), "%d,%f,%f%c", &state, &rate, &elapsed, &trailing) != 3 ||
			state < 0 || state > 3 || !std::isfinite(rate) || !std::isfinite(elapsed) || elapsed < 0) { return false; }
		music.playing = state != 0;
		return true;
	}
	if (data[0] != 2 || (data[1] != 0 && data[1] != 2)) { return false; }
	String text;
	size_t limit = data[1] == 2 ? 160 : 96;
	for (size_t i = 3; i < size && i - 3 < limit; ++i) {
		if (!data[i]) { return false; }
		text += char(data[i]);
	}
	if (data[1] == 2) { music.title = text; }
	else { music.artist = text; }
	return true;
}

/** @brief Restrict controls to commands advertised by the active player. @param data IDs. @param size Bytes. @return Supported action mask. */
inline uint8_t music_commands(const uint8_t *data, size_t size) {
	uint8_t mask = 0;
	if (!data) { return mask; }
	for (size_t i = 0; i < size; ++i) {
		if (data[i] >= 2 && data[i] <= 6) { mask |= uint8_t(1U << data[i]); }
	}
	return mask;
}

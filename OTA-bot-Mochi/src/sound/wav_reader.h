/**
 * @file wav_reader.h
 * @brief Bounded RIFF PCM validation for short 16 kHz SD sound effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

/** @brief Validated PCM range, independent of filesystem and struct padding. */
struct WavInfo {
	uint32_t offset = 0, bytes = 0;
	uint16_t channels = 0;
};

/** @brief Decode one unaligned little-endian integer. @param p Source bytes. @return Value. */
inline uint32_t wav_u32(const uint8_t *p) {
	return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

/** @brief Accept signed 16-bit PCM mono/stereo at the codec's fixed rate. @param p Format bytes. @return Channels or zero. */
inline uint16_t wav_channels(const uint8_t *p) {
	uint16_t channels = p[2];
	if (p[0] != 1 || p[1] || p[3] || (channels != 1 && channels != 2)) { return 0; }
	if (wav_u32(p + 4) != 16000 || wav_u32(p + 8) != 32000U * channels ||
		p[12] != 2 * channels || p[13] || p[14] != 16 || p[15]) { return 0; }
	return channels;
}

/** @brief Validate chunks with overflow-safe bounds and at most three seconds of audio. @param file Seekable reader. @param out Validated range. @return Supported complete WAV. */
template<class Reader>
bool wav_open(Reader &file, WavInfo &out) {
	uint8_t header[16];
	if (!file.seek(0) || file.read(header, 12) != 12 || memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4)) { return false; }
	uint64_t end = uint64_t(wav_u32(header + 4)) + 8;
	if (end < 12 || end > file.size()) { return false; }
	WavInfo next;
	uint32_t offset = 12;
	for (uint8_t chunks = 0; chunks < 32 && uint64_t(offset) + 8 <= end; ++chunks) {
		if (!file.seek(offset) || file.read(header, 8) != 8) { return false; }
		uint32_t size = wav_u32(header + 4);
		uint64_t after = uint64_t(offset) + 8 + size + (size & 1U);
		if (after > end || after > UINT32_MAX) { return false; }
		if (!memcmp(header, "fmt ", 4)) {
			if (size < 16 || file.read(header, 16) != 16 || !(next.channels = wav_channels(header))) { return false; }
		} else if (!memcmp(header, "data", 4)) {
			next.offset = offset + 8; next.bytes = size;
		}
		if (next.channels && next.offset) {
			if (!next.bytes || next.bytes % (2 * next.channels) || next.bytes > 96000U * next.channels) { return false; }
			out = next;
			return file.seek(next.offset);
		}
		offset = uint32_t(after);
	}
	return false;
}

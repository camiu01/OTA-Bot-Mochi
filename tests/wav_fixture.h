/**
 * @file wav_fixture.h
 * @brief Explicit little-endian WAV fixture creation for decoder and worker tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>

/** @brief Write one fixture integer. @param bytes Buffer. @param at Offset. @param value Number. */
inline void wav_fixture_u32(std::vector<uint8_t> &bytes, size_t at, uint32_t value) {
	for (uint8_t i = 0; i < 4; ++i) { bytes[at + i] = uint8_t(value >> (8 * i)); }
}

/** @brief Generate 16 kHz PCM with distinguishable stereo channels. @param channels Mono/stereo. @param frames Length. @return WAV bytes. */
inline std::vector<uint8_t> wav_fixture(uint16_t channels = 1, uint32_t frames = 1600) {
	std::vector<uint8_t> bytes(44 + frames * channels * 2, 0);
	memcpy(bytes.data(), "RIFF", 4); memcpy(bytes.data() + 8, "WAVEfmt ", 8);
	wav_fixture_u32(bytes, 4, uint32_t(bytes.size() - 8));
	wav_fixture_u32(bytes, 16, 16); bytes[20] = 1; bytes[22] = uint8_t(channels);
	wav_fixture_u32(bytes, 24, 16000); wav_fixture_u32(bytes, 28, 32000 * channels);
	bytes[32] = uint8_t(2 * channels); bytes[34] = 16;
	memcpy(bytes.data() + 36, "data", 4); wav_fixture_u32(bytes, 40, uint32_t(bytes.size() - 44));
	for (uint32_t i = 0; i < frames; ++i) {
		for (uint16_t c = 0; c < channels; ++c) {
			int16_t sample = c ? -20000 : 20000;
			size_t at = 44 + (i * channels + c) * 2;
			bytes[at] = uint8_t(sample); bytes[at + 1] = uint8_t(uint16_t(sample) >> 8);
		}
	}
	return bytes;
}

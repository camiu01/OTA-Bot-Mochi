/**
 * @file pcm_sequence.h
 * @brief Sample-clocked, bounded chirps independent of display frame timing.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "pcm_tone.h"

struct SoundNote { uint16_t hz, ms; };

class PcmSequence {
	SoundNote notes[5] = {};
	PcmTone oscillator;
	uint8_t count = 0, index = 0, level = 0;
	uint32_t position = 0;
public:
	/** @brief Validate at most one second of supported notes. @param input Notes. @param size Count. @param percent Amplitude. @return Valid. */
	static bool valid(const SoundNote *input, uint8_t size, uint8_t percent) {
		if (!input || !size || size > 5 || percent > 30) { return false; }
		uint32_t duration = 0;
		for (uint8_t i = 0; i < size; ++i) {
			if (!input[i].ms || (input[i].hz && (input[i].hz < 300 || input[i].hz > 4000))) { return false; }
			duration += input[i].ms;
		}
		return duration <= 1000;
	}
	/** @brief Copy a validated cue before rendering. @param input Notes. @param size Count, zero to stop. @param percent Amplitude. */
	void start(const SoundNote *input, uint8_t size, uint8_t percent) {
		count = size; index = 0; position = 0; level = percent;
		oscillator = PcmTone{};
		for (uint8_t i = 0; i < size; ++i) { notes[i] = input[i]; }
	}
	/** @brief Report unrendered notes, including internal pauses. @return Playing. */
	bool playing() const { return index < count; }
	/** @brief Render exact note lengths and 5ms sample-level edges. @param out Stereo samples. @param frames Stereo frame count. */
	void render(int16_t *out, size_t frames) {
		for (size_t i = 0; i < frames; ++i) {
			uint16_t hz = playing() ? notes[index].hz : 0;
			oscillator.render(out + 2 * i, 1, hz, level);
			if (!playing()) { continue; }
			uint32_t length = uint32_t(notes[index].ms) * 16;
			uint32_t edge = position < 80 ? position : 80;
			if (length - position - 1 < edge) { edge = length - position - 1; }
			out[2 * i] = out[2 * i + 1] = int32_t(out[2 * i]) * edge / 80;
			if (++position == length) { ++index; position = 0; }
		}
	}
};

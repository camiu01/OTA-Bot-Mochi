/**
 * @file pcm_tone.h
 * @brief Bounded 16 kHz signed PCM oscillator, duplicated into both I2S slots.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

class PcmTone {
	uint32_t phase = 0;
public:
	/** @brief Fill stereo frames continuously. @param out Output pairs. @param frames Pair count. @param hz Hertz. @param percent Amplitude 0..30. */
	void render(int16_t *out, size_t frames, uint16_t hz, uint8_t percent) {
		static constexpr int16_t sine[] = {
			0, 6393, 12539, 18204, 23170, 27245, 30273, 32137,
			32767, 32137, 30273, 27245, 23170, 18204, 12539, 6393,
			0, -6393, -12539, -18204, -23170, -27245, -30273, -32137,
			-32767, -32137, -30273, -27245, -23170, -18204, -12539, -6393
		};
		if (percent > 30 || hz < 300 || hz > 4000) { percent = 0; }
		uint32_t step = uint32_t((uint64_t(hz) << 32) / 16000);
		for (size_t i = 0; i < frames; ++i) {
			int16_t sample = int32_t(sine[phase >> 27]) * percent / 100;
			out[2 * i] = out[2 * i + 1] = sample;
			phase += step;
		}
		if (!percent) { phase = 0; }
	}
};

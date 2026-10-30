/**
 * @file sd_audio.h
 * @brief Worker-only, read-only SDMMC WAV playback with atomic last-attempt diagnostics.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../../pin_config.h"
#include "wav_reader.h"
#include "sound.h"
#include "../diagnostics/device_status.h"
#include <SD_MMC.h>
#include <cstdio>
#include <esp_log.h>

/** @brief Confine card mounting, file handles, and PCM decoding to the audio worker. */
class SdAudio {
public:
	/** @brief Open a fixed cue or reaction-family path lazily, with generic face fallback; never format a card. @param cue SoundCue ordinal. @return Valid file ready. */
	bool start(uint8_t cue) {
		stop();
		static const char *const paths[] = {"ambient", "mood", "tap", "face", "notification", "rest", "done", "call", "test", "startup", "charging"};
		static const char *const families[] = {"face-bright", "face-warm", "face-soft", "face-bounce", "face-flat"};
		static_assert(sizeof(paths) / sizeof(paths[0]) == uint8_t(SoundCue::Count), "Keep SD cue names aligned");
		static_assert(sizeof(families) / sizeof(families[0]) == uint8_t(FaceTone::Count), "Keep SD reaction families aligned");
		bool family = cue >= SOUND_SD_FACE_BASE && cue < SOUND_SD_FACE_BASE + uint8_t(FaceTone::Count);
		if (!family && cue >= sizeof(paths) / sizeof(paths[0])) { return false; }
		if (!_mounted) {
			if (!SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA) || !SD_MMC.begin("/sdcard", true, false)) {
				ESP_LOGW("sd-audio", "Card unavailable; using built-in cue");
				device_status::sd = device_status::SdState::Missing;
				SD_MMC.end(); return false;
			}
			_mounted = true;
		}
		if (family && open_file(families[cue - SOUND_SD_FACE_BASE])) { device_status::sd = device_status::SdState::Ready; return true; }
		if (open_file(paths[family ? uint8_t(SoundCue::Face) : cue])) { device_status::sd = device_status::SdState::Ready; return true; }
		ESP_LOGW("sd-audio", "Missing or unsupported WAV; using built-in cue");
		device_status::sd = device_status::SdState::Error;
		release(); return false;
	}

	/** @brief Close the current clip on cancellation or completion. */
	void stop() { if (_file) { _file.close(); } _remaining = 0; }
	/** @brief Unmount after disabling SD sounds; called only by the audio worker. */
	void release() { stop(); if (_mounted) { SD_MMC.end(); _mounted = false; } }
	/** @brief Read playback state without touching the filesystem. @return Data remains. */
	bool playing() const { return _remaining != 0; }

	/** @brief Fill a bounded stereo block with scaled PCM and short edge fades. @param out Stereo buffer. @param frames Frames, at most 80. @param percent Amplitude 0..30. @return Read succeeded. */
	bool render(int16_t *out, size_t frames, uint8_t percent) {
		if (!out || frames > 80 || percent > 30) { return false; }
		memset(out, 0, frames * 2 * sizeof(int16_t));
		if (!playing()) { return true; }
		uint8_t input[320];
		size_t frame_bytes = 2 * _info.channels;
		size_t count = _remaining / frame_bytes;
		if (count > frames) { count = frames; }
		if (_file.read(input, count * frame_bytes) != count * frame_bytes) {
			ESP_LOGW("sd-audio", "WAV read failed; stopping clip");
			device_status::sd = device_status::SdState::Error;
			release(); return false;
		}
		uint32_t position = (_info.bytes - _remaining) / frame_bytes;
		uint32_t total = _info.bytes / frame_bytes;
		for (size_t i = 0; i < count; ++i) {
			uint32_t edge = position + i < 80 ? position + i : 80;
			if (total - position - i - 1 < edge) { edge = total - position - i - 1; }
			for (uint8_t channel = 0; channel < 2; ++channel) {
				size_t at = i * frame_bytes + (channel < _info.channels ? channel : 0) * 2;
				int32_t sample = uint16_t(input[at]) | (uint16_t(input[at + 1]) << 8);
				if (sample >= 32768) { sample -= 65536; }
				out[2 * i + channel] = sample * int32_t(percent) * int32_t(edge) / 8000;
			}
		}
		_remaining -= count * frame_bytes;
		if (!_remaining) { _file.close(); }
		return true;
	}
private:
	/** @brief Validate a fixed basename without exposing arbitrary file paths. @param name Known cue name. @return Supported WAV ready. */
	bool open_file(const char *name) {
		stop();
		char path[48];
		snprintf(path, sizeof(path), "/mochi/sounds/%s.wav", name);
		_file = SD_MMC.open(path, FILE_READ);
		if (!_file || !wav_open(_file, _info)) { stop(); return false; }
		_remaining = _info.bytes;
		return true;
	}
	File _file;
	WavInfo _info;
	uint32_t _remaining = 0;
	bool _mounted = false;
};

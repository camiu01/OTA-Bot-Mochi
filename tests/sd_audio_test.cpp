/**
 * @file sd_audio_test.cpp
 * @brief WAV validation, bounded decoding, SD fallback, and cancellation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/sound/sd_audio.h"
#include "wav_fixture.h"
#include <cassert>
#include <iostream>

/** @brief Check malformed and oversized RIFF data without allowing partial output. */
void wav_checks() {
	for (uint16_t channels : {1, 2}) {
		File file; file.data = std::make_shared<std::vector<uint8_t>>(wav_fixture(channels));
		WavInfo info;
		assert(wav_open(file, info) && info.channels == channels && info.offset == 44);
	}
	for (int invalid = 0; invalid < 8; ++invalid) {
		auto bytes = wav_fixture();
		if (invalid == 0) { bytes[0] = 'X'; }
		if (invalid == 1) { bytes[20] = 3; }
		if (invalid == 2) { bytes[22] = 3; }
		if (invalid == 3) { bytes[34] = 8; }
		if (invalid == 4) { wav_fixture_u32(bytes, 24, 44100); }
		if (invalid == 5) { wav_fixture_u32(bytes, 40, UINT32_MAX); }
		if (invalid == 6) { bytes.pop_back(); }
		if (invalid == 7) { bytes = wav_fixture(1, 48001); }
		File file; file.data = std::make_shared<std::vector<uint8_t>>(bytes);
		WavInfo info{77, 88, 99};
		assert(!wav_open(file, info) && info.offset == 77);
	}
}

/** @brief Check absent cards, actual PCM scale, completion, and read failure. */
void sd_checks() {
	SdAudio sd;
	assert(!sd.start(255) && SD_MMC.mounts == 0);
	assert(!sd.start(2) && !sd.playing());
	assert(device_status::sd == device_status::SdState::Missing);
	SD_MMC.present = true;
	SD_MMC.files["/mochi/sounds/tap.wav"] = wav_fixture(2, 160);
	// A failed mount backs off, so a missing card cannot stall every cue.
	assert(!sd.start(2) && SD_MMC.mounts == 1);
	test_now += 11000;
	assert(sd.start(2) && sd.playing());
	assert(device_status::sd == device_status::SdState::Ready);
	assert(SD_MMC.clk == 2 && SD_MMC.cmd == 1 && SD_MMC.data == 3);
	int16_t samples[160];
	assert(sd.render(samples, 80, 30));
	assert(samples[0] == 0 && samples[158] > 0 && samples[159] < 0);
	assert(sd.render(samples, 80, 30) && !sd.playing() && samples[158] == 0);
	// A missing file keeps the card mounted instead of forcing a remount.
	unsigned mounts = SD_MMC.mounts;
	assert(!sd.start(4) && SD_MMC.mounts == mounts);
	assert(device_status::sd == device_status::SdState::Error);
	assert(sd.start(2)); File::short_read = true;
	assert(!sd.render(samples, 80, 10) && !sd.playing()); File::short_read = false;
	assert(device_status::sd == device_status::SdState::Error);
	assert(sd.start(2)); sd.release(); assert(!sd.playing());
}

/** @brief Specific reaction files fall back to face.wav before the built-in chirp. */
void cue_file_checks() {
	SdAudio sd;
	SD_MMC.files["/mochi/sounds/face.wav"] = wav_fixture();
	assert(sd.start(SOUND_SD_FACE_BASE + uint8_t(FaceTone::Warm)));
	assert(SD_MMC.last_path == "/mochi/sounds/face.wav");
	SD_MMC.files["/mochi/sounds/face-warm.wav"] = wav_fixture();
	assert(sd.start(SOUND_SD_FACE_BASE + uint8_t(FaceTone::Warm)));
	assert(SD_MMC.last_path == "/mochi/sounds/face-warm.wav");
	SD_MMC.files["/mochi/sounds/face-warm.wav"] = {1, 2, 3};
	assert(sd.start(SOUND_SD_FACE_BASE + uint8_t(FaceTone::Warm)));
	assert(SD_MMC.last_path == "/mochi/sounds/face.wav");
	for (const char *name : {"startup", "charging"}) {
		std::string path = std::string("/mochi/sounds/") + name + ".wav";
		SD_MMC.files[path] = wav_fixture();
		assert(sd.start(uint8_t(name[0] == 's' ? SoundCue::Startup : SoundCue::Charging)));
		assert(SD_MMC.last_path == path);
	}
	sd.release();
}

/** @brief Run WAV and SD decoder regressions. @return Success. */
int main() {
	wav_checks(); sd_checks(); cue_file_checks();
	std::cout << "Read-only SD WAV boundaries, decoding and cancellation passed\n";
	return 0;
}

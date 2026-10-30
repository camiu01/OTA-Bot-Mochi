/**
 * @file es8311_test.cpp
 * @brief Codec configuration, PCM bounds, mute, failure cleanup, and DMA diagnostics.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/sound/buzzer_output.h"
#include "../OTA-bot-Mochi/src/sound/pcm_tone.h"
#include "../OTA-bot-Mochi/src/sound/es8311_output.h"
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <driver/i2s_std.h>
#include <freertos/task.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <SD_MMC.h>
#include "wav_fixture.h"

static CompanionSettings profile;
/** @brief Supply main-task settings without linking unrelated renderers. @return Test settings. */
const CompanionSettings &settings_get() { return profile; }
/** @brief Supply manual-night state to the isolated codec fixture. @return Night mode. */
bool settings_night() { return profile.night; }

/** @brief Run a bounded portion of the real PCM worker with fake DMA. */
static void pump(int blocks = 1) {
	test_i2s_remaining = blocks;
	try { test_audio_task(nullptr); } catch (const TestAudioYield &) {}
}

/** @brief Exact sample lengths, copied notes, edges, and invalid sequence bounds. */
static void sequence_tests() {
	SoundNote notes[] = {{1800, 55}, {0, 25}, {2400, 65}};
	assert(PcmSequence::valid(notes, 3, 10));
	assert(!PcmSequence::valid(nullptr, 3, 10) && !PcmSequence::valid(notes, 0, 10));
	assert(!PcmSequence::valid(notes, 6, 10) && !PcmSequence::valid(notes, 3, 31));
	SoundNote bad[] = {{299, 10}, {1800, 0}, {1800, 1001}};
	for (const auto &note : bad) { assert(!PcmSequence::valid(&note, 1, 10)); }
	SoundNote long_notes[] = {{1800, 600}, {2400, 600}};
	assert(!PcmSequence::valid(long_notes, 2, 10));
	PcmSequence sequence;
	sequence.start(notes, 3, 10);
	notes[0].hz = 0;
	int16_t samples[4640];
	sequence.render(samples, 2320);
	assert(!sequence.playing());
	assert(samples[0] == 0 && samples[1758] == 0 && samples[4638] == 0);
	assert(std::any_of(samples, samples + 1760, [](int16_t v) { return v != 0; }));
	assert(std::all_of(samples + 1760, samples + 2560, [](int16_t v) { return v == 0; }));
	assert(std::any_of(samples + 2560, samples + 4640, [](int16_t v) { return v != 0; }));
	sequence.render(samples, 2320);
	assert(std::all_of(std::begin(samples), std::end(samples), [](int16_t v) { return v == 0; }));
}

/** @brief Every supported pitch has bounded, identical stereo slots; invalid input is silent. */
static void pcm_tests() {
	for (uint16_t hz : {300, 800, 1800, 2400, 4000}) {
		PcmTone tone;
		int16_t samples[3200];
		tone.render(samples, 1600, hz, 30);
		int crossings = 0;
		for (size_t i = 0; i < 1600; ++i) {
			assert(samples[2 * i] == samples[2 * i + 1]);
			assert(std::abs(int(samples[2 * i])) <= 9830);
			if (i && samples[2 * (i - 1)] <= 0 && samples[2 * i] > 0) { ++crossings; }
		}
		assert(std::abs(crossings - int(hz / 10)) <= 1);
		for (auto input : {std::pair<uint16_t, uint8_t>{0, 30}, {299, 10}, {4001, 10}, {2000, 31}, {2000, 0}}) {
			tone.render(samples, 1600, input.first, input.second);
			assert(std::all_of(std::begin(samples), std::end(samples), [](int16_t v) { return v == 0; }));
		}
	}
	// Chunk boundaries must not reset oscillator phase.
	PcmTone whole, split;
	int16_t a[320], b[320];
	whole.render(a, 160, 1700, 10);
	split.render(b, 80, 1700, 10);
	split.render(b + 160, 80, 1700, 10);
	assert(std::equal(std::begin(a), std::end(a), std::begin(b)));
}

/** @brief Partial initialization failures leave no channel or unmuted amplifier. */
static void failure_tests() {
	buzzer_end();
	assert(test_gpio_writes[AUDIO_PA] == 0);
	assert(!buzzer_write(1800, 10));
	for (int stage : {1, 2, 3}) {
		test_i2s_failure = stage;
		assert(!buzzer_begin() && test_i2s_allocated == 0);
		assert(test_gpio_levels[AUDIO_PA] == LOW && test_audio_tasks == 0);
	}
	test_i2s_failure = 0;
	TwoWire &wire = i2c_bus();
	wire.present[0x18] = true;
	wire.fail_write_register = 0x12;
	assert(!buzzer_begin() && test_i2s_allocated == 0);
	wire.fail_write_register = -1;
	test_task_failure = true;
	assert(!buzzer_begin() && test_i2s_allocated == 0);
	test_task_failure = false;
}

/** @brief Verify reference clocks/pins, silence, expiration, and asynchronous error propagation. */
static void playback_tests() {
	assert(buzzer_begin() && test_audio_tasks == 1 && test_i2s_allocated == 1);
	const auto &format = test_i2s_standard;
	assert(format.clk_cfg.sample_rate_hz == 16000 && format.clk_cfg.mclk_multiple == 256);
	assert(format.slot_cfg.data_bit_width == 16 && format.slot_cfg.slot_mode == I2S_SLOT_MODE_STEREO);
	assert(format.gpio_cfg.mclk == 16 && format.gpio_cfg.bclk == 9 && format.gpio_cfg.ws == 45);
	assert(format.gpio_cfg.dout == 8 && format.gpio_cfg.din == I2S_GPIO_UNUSED);
	assert(test_i2s_config.auto_clear && test_i2s_config.dma_frame_num == 80);
	const auto &registers = i2c_bus().registers[0x18];
	assert(registers[0x00] == 0x80 && registers[0x01] == 0x3F && registers[0x08] == 0xFF);
	assert(registers[0x09] == 0x0C && registers[0x32] == 0xD8);
	assert(test_gpio_levels[AUDIO_PA] == LOW);
	assert(!buzzer_write(299, 10) && !buzzer_write(4001, 10) && !buzzer_write(1800, 31));
	test_now = UINT32_MAX - 10;
	assert(buzzer_write(1800, 10) && test_gpio_levels[AUDIO_PA] == HIGH);
	test_now = 5; pump();
	assert(std::any_of(test_i2s_samples.begin(), test_i2s_samples.end(), [](int16_t v) { return v != 0; }));
	test_now = 100; pump();
	assert(std::all_of(test_i2s_samples.begin(), test_i2s_samples.end(), [](int16_t v) { return v == 0; }));
	buzzer_end();
	assert(test_gpio_levels[AUDIO_PA] == LOW);
	pump();
	assert(std::all_of(test_i2s_samples.begin(), test_i2s_samples.end(), [](int16_t v) { return v == 0; }));
	for (bool short_write : {false, true}) {
		assert(buzzer_begin() && buzzer_write(2400, 10));
		test_i2s_failure = short_write ? 0 : 4;
		test_i2s_short_write = short_write;
		pump();
		assert(!buzzer_healthy() && !buzzer_write(2400, 10));
		buzzer_end();
		assert(test_gpio_levels[AUDIO_PA] == LOW);
		test_i2s_failure = 0; test_i2s_short_write = false;
	}
	assert(buzzer_begin() && buzzer_healthy() && test_audio_tasks == 1 && test_i2s_allocated == 1);
	buzzer_end();
}

/** @brief The real sound service propagates worker failures and respects master/night mute. */
static void service_tests() {
	UiPageEvents events;
	profile.sound = false;
	sound_init(0);
	sound_request(SoundCue::Test, 0);
	sound_poll(0, events, false, false, false);
	assert(sound_status() == SoundStatus::Off);
	profile.sound = profile.night = profile.sound_night_mute = true;
	sound_poll(10, events, false, false, false);
	assert(sound_status() == SoundStatus::Night && test_gpio_levels[AUDIO_PA] == LOW);
	profile.night = false;
	assert(sound_status() == SoundStatus::Armed);
	sound_request(SoundCue::Test, 1000);
	sound_poll(1000, events, false, false, false);
	test_now = 1010; sound_poll(1010, events, false, false, false);
	assert(sound_status() == SoundStatus::Ready && strcmp(sound_status_name(), "i2s-ready") == 0);
	assert(test_gpio_levels[AUDIO_PA] == HIGH);
	test_i2s_failure = 4; pump();
	assert(sound_status() == SoundStatus::Error && strcmp(sound_status_name(), "i2s-error") == 0);
	sound_poll(1020, events, false, false, false);
	assert(test_gpio_levels[AUDIO_PA] == LOW);
	test_i2s_failure = 0;
	sound_request(SoundCue::Test, 3000);
	sound_poll(3000, events, false, false, false);
	assert(sound_status() == SoundStatus::Error);
	profile.sound = false; sound_poll(3010, events, false, false, false);
	profile.sound = true;
	sound_request(SoundCue::Test, 3100);
	sound_poll(3100, events, false, false, false);
	test_now = 3110; sound_poll(3110, events, false, false, false);
	assert(sound_status() == SoundStatus::Ready && test_gpio_levels[AUDIO_PA] == HIGH);
	profile.night = true; sound_poll(3120, events, false, false, false);
	assert(sound_status() == SoundStatus::Night && test_gpio_levels[AUDIO_PA] == LOW);
	assert(test_audio_tasks == 1 && test_i2s_allocated == 1);
}

static uint32_t next_poll = 0, poll_interval = 0;
static UiPageEvents delayed_events{};
/** @brief Simulate expensive frames while the independent worker continues DMA. */
static void delayed_poll() {
	if (test_now < next_poll) { return; }
	next_poll += poll_interval;
	sound_poll(test_now, delayed_events, false, false, false);
}

/** @brief Short cues and the long test retain both notes despite 80/160ms main-loop stalls. */
static void delayed_service_tests() {
	for (uint32_t interval : {80, 160}) {
		for (SoundCue cue : {SoundCue::Tap, SoundCue::Face, SoundCue::Test}) {
			profile.sound = profile.sound_faces = true;
			profile.night = profile.sound_ambient = false; profile.sound_level = 10;
			test_now = 10000; sound_init(test_now);
			sound_request(cue, test_now);
			sound_poll(test_now, delayed_events, false, false, false);
			assert(es8311_playing());
			test_i2s_history.clear();
			poll_interval = interval; next_poll = test_now + interval;
			test_i2s_hook = delayed_poll;
			pump(140);
			test_i2s_hook = nullptr;
			size_t second_ms = cue == SoundCue::Test ? 330 : 80;
			auto middle = test_i2s_history.begin() + second_ms * 32;
			assert(std::any_of(test_i2s_history.begin(), middle, [](int16_t v) { return v != 0; }));
			assert(std::any_of(middle, test_i2s_history.end(), [](int16_t v) { return v != 0; }));
			assert(!es8311_playing() && test_gpio_levels[AUDIO_PA] == LOW);
			sound_poll(test_now, delayed_events, false, false, false);
			assert(sound_status() == SoundStatus::Ready);
		}
	}
}

/** @brief The amplifier drains buffered PCM, but explicit mute cancels immediately. */
static void sequence_mute_tests() {
	SoundNote note{1800, 10};
	assert(es8311_play(&note, 1, 10));
	pump(2);
	assert(test_gpio_levels[AUDIO_PA] == HIGH && es8311_playing());
	pump(8);
	assert(test_gpio_levels[AUDIO_PA] == LOW && !es8311_playing());
	assert(es8311_play(&note, 1, 10));
	es8311_end();
	assert(test_gpio_levels[AUDIO_PA] == LOW && !es8311_playing());
	pump(8);
	assert(test_gpio_levels[AUDIO_PA] == LOW);
	assert(std::all_of(test_i2s_samples.begin(), test_i2s_samples.end(), [](int16_t v) { return v == 0; }));
}

/** @brief SD access is opt-in, respects master/night mute, and cancels when disabled. */
static void sd_service_tests() {
	assert(SD_MMC.mounts == 0 && SD_MMC.opens == 0);
	UiPageEvents events;
	profile = CompanionSettings{}; profile.sound_sd = true;
	test_now = 30000; sound_init(test_now);
	sound_request(SoundCue::Test, test_now); sound_poll(test_now, events, false, false, false);
	pump(); assert(SD_MMC.mounts == 0);
	profile.sound = profile.night = true;
	sound_request(SoundCue::Test, test_now); sound_poll(test_now, events, false, false, false);
	pump(); assert(SD_MMC.mounts == 0);
	profile.night = false;
	SD_MMC.present = true; SD_MMC.files["/mochi/sounds/test.wav"] = wav_fixture(1, 16000);
	sound_request(SoundCue::Test, test_now); sound_poll(test_now, events, false, false, false);
	pump(3); assert(SD_MMC.mounts == 1 && SD_MMC.opens == 1 && es8311_playing());
	assert(test_i2s_samples[80] == 2000 && test_i2s_samples[81] == 2000);
	profile.sound_sd = false; sound_poll(test_now, events, false, false, false);
	assert(test_gpio_levels[AUDIO_PA] == LOW && !es8311_playing());
	pump(); assert(SD_MMC.unmounts == 1);
	sound_request(SoundCue::Test, test_now + 2000); sound_poll(test_now + 2000, events, false, false, false);
	pump(3); assert(SD_MMC.opens == 1);
	profile.sound = false; sound_poll(test_now, events, false, false, false); pump();
}

/** @brief The real service selects startup, charging and five reaction-family files. */
static void sd_family_tests() {
	profile = CompanionSettings{}; profile.sound = profile.sound_sd = true;
	UiPageEvents events;
	const char *names[] = {"face-bright", "face-warm", "face-soft", "face-bounce", "face-flat"};
	for (uint8_t i = 0; i < uint8_t(FaceTone::Count); ++i) {
		std::string path = std::string("/mochi/sounds/") + names[i] + ".wav";
		SD_MMC.files[path] = wav_fixture();
		test_now += 2000; sound_init(test_now);
		sound_request_face(FaceTone(i), test_now); sound_poll(test_now, events, false, false, false);
		pump(3); assert(SD_MMC.last_path == path && es8311_playing());
	}
	for (SoundCue cue : {SoundCue::Startup, SoundCue::Charging}) {
		std::string path = std::string("/mochi/sounds/") + (cue == SoundCue::Startup ? "startup.wav" : "charging.wav");
		SD_MMC.files[path] = wav_fixture();
		test_now += 2000; sound_init(test_now);
		assert(sound_request(cue, test_now)); sound_poll(test_now, events, false, false, false);
		pump(3); assert(SD_MMC.last_path == path && es8311_playing());
	}
	sound_init(test_now); pump();
}

/** @brief Run without physical audio output. @return Success. */
int main() {
	pcm_tests(); sequence_tests(); failure_tests(); playback_tests(); service_tests();
	delayed_service_tests(); sequence_mute_tests(); sd_service_tests(); sd_family_tests();
	std::cout << "ES8311 codec, PCM sequences, delayed rendering, DMA drain, mute, and failures passed\n";
}

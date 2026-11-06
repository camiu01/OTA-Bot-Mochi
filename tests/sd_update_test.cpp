/**
 * @file sd_update_test.cpp
 * @brief Manual SD firmware confirmation, ownership, compatibility, bounded I/O and inactive-slot failures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/sd_update.h"
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/update/ota_image.h"
#include "../OTA-bot-Mochi/src/sound/sd_audio.h"
#include "wav_fixture.h"
#include <Arduino.h>
#include <esp_ota_ops.h>
#include <cassert>
#include <iostream>
#include <vector>

static OtaStatus web;
static unsigned silenced = 0;
/** @brief Observe maintenance mute without initializing the hardware backend. */
void sound_init(uint32_t) { ++silenced; }
/** @brief Isolate the updater from HTTP while checking maintenance exclusion. */
const OtaStatus &ota_status() { return web; }

#if MOCHI_HAS_SD_AUDIO
/** @brief Fake application with a board marker crossing a 4096-byte read boundary. */
static std::vector<uint8_t> binary() {
	std::vector<uint8_t> bytes(10000, 0);
	bytes[0] = 0xe9; bytes[12] = 9;
	bytes[32] = 0x32; bytes[33] = 0x54; bytes[34] = 0xcd; bytes[35] = 0xab;
	memcpy(bytes.data() + 4090, ota_image_marker(), strlen(ota_image_marker()));
	return bytes;
}
/** @brief Start with a mounted-capable card and no injected flash/transport failures. */
static void reset(uint32_t now = 100) {
	sd_update_stop();
	assert(sd_owner.load() == SdOwner::None);
	SD_MMC.present = true; SD_MMC.files.clear();
	SD_MMC.files[sd_update_path()] = binary();
	File::short_read = false;
	test_no_slot = false;
	test_running = {0x10000, 0x640000}; test_target = {0x650000, 0x640000};
	test_begin_error = test_write_error = test_end_error = test_select_error = 0;
	test_now = now;
	assert(sd_update_begin(now));
}
/** @brief Poll bounded steps until checking/writing stops, never auto-confirming. */
static void transfer() {
	for (unsigned i = 0; i < 10; ++i) {
		auto phase = sd_update_status().phase;
		if (phase != SdUpdatePhase::Waiting && phase != SdUpdatePhase::Checking && phase != SdUpdatePhase::Writing) { return; }
		size_t checked = sd_update_status().checked, written = sd_update_status().written;
		sd_update_poll(++test_now);
		assert(sd_update_status().checked - checked <= 4096);
		assert(sd_update_status().written - written <= 4096);
	}
	assert(false);
}
/** @brief Reject unsafe source files and legacy/missing partitions without beginning any flash write. */
static void input_checks() {
	int begins = test_begins, selects = test_selects;
	for (size_t size : {size_t(0), size_t(287), size_t(0x640001)}) {
		reset(); SD_MMC.files[sd_update_path()] = std::vector<uint8_t>(size);
		transfer(); assert(sd_update_status().error);
	}
	reset(); SD_MMC.files.clear(); transfer(); assert(sd_update_status().error);
	reset(); SD_MMC.present = false; transfer(); assert(sd_update_status().error);
	reset(); SD_MMC.files[sd_update_path()][0] = 0; transfer(); assert(sd_update_status().error);
	reset(); SD_MMC.files[sd_update_path()][12] = 0; transfer(); assert(sd_update_status().error);
	reset(); SD_MMC.files[sd_update_path()][32] = 0; transfer(); assert(sd_update_status().error);
	reset(); SD_MMC.files[sd_update_path()][4090] = 'X'; transfer(); assert(sd_update_status().error);
	for (int invalid = 0; invalid < 4; ++invalid) {
		sd_update_stop();
		if (invalid == 0) { test_no_slot = true; }
		if (invalid == 1) { test_target.size = 0x140000; }
		if (invalid == 2) { test_target.address = test_running.address; }
		if (invalid == 3) { test_running.address = 0x20000; }
		assert(sd_update_begin(++test_now) && sd_update_status().error);
		test_no_slot = false; test_target = {0x650000, 0x640000}; test_running = {0x10000, 0x640000};
	}
	assert(test_begins == begins && test_selects == selects);
	sd_update_stop();
	web.active = true; unsigned muted = silenced;
	assert(!sd_update_begin(100) && silenced == muted && !sd_update_status().active);
	web.active = false;
}
/** @brief Audio and updater must never share the SD peripheral or unmount each other's lease. */
static void ownership_checks() {
	sd_update_stop();
	SdAudio audio; SD_MMC.present = true;
	SD_MMC.files["/mochi/sounds/tap.wav"] = wav_fixture();
	assert(audio.start(uint8_t(SoundCue::Tap)));
	assert(sd_owner.load() == SdOwner::Audio);
	unsigned mounts = SD_MMC.mounts, unmounts = SD_MMC.unmounts;
	assert(sd_update_begin(100));
	sd_update_poll(101);
	assert(SD_MMC.mounts == mounts && sd_update_status().phase == SdUpdatePhase::Waiting);
	sd_update_poll(10100);
	assert(sd_update_status().error && SD_MMC.unmounts == unmounts);
	assert(sd_owner.load() == SdOwner::Audio);
	sd_update_stop(); assert(sd_owner.load() == SdOwner::Audio);
	audio.release(); reset();
	transfer();
	assert(sd_update_status().phase == SdUpdatePhase::Ready);
	assert(sd_owner.load() == SdOwner::Update);
	mounts = SD_MMC.mounts;
	assert(!audio.start(uint8_t(SoundCue::Tap)) && SD_MMC.mounts == mounts);
	audio.release(); assert(sd_owner.load() == SdOwner::Update);
	sd_update_stop(); assert(sd_owner.load() == SdOwner::None);
}
/** @brief Require a fresh confirmation every session, and support safe cancellation between chunks. */
static void confirmation_checks() {
	reset(); int begins = test_begins, selects = test_selects;
	assert(!sd_update_confirm());
	transfer(); assert(sd_update_status().phase == SdUpdatePhase::Ready);
	sd_update_poll(++test_now);
	assert(test_begins == begins && !sd_update_status().written);
	sd_update_stop(); reset(); transfer();
	assert(sd_update_status().phase == SdUpdatePhase::Ready && test_begins == begins);
	assert(sd_update_confirm() && !sd_update_confirm());
	sd_update_poll(++test_now);
	assert(sd_update_status().written == 4096 && test_begins == begins + 1);
	int aborts = test_aborts;
	sd_update_stop();
	assert(test_aborts == aborts + 1 && test_selects == selects && !sd_update_status().active);
	reset(); transfer();
	sd_update_poll(300100);
	assert(sd_update_status().error && test_selects == selects);
}
/** @brief Reject read, mutation, flash, finalization and boot-selection failures without rebooting. */
static void failure_checks() {
	int selects = test_selects;
	reset(); sd_update_poll(++test_now); File::short_read = true;
	transfer(); assert(sd_update_status().error && !sd_update_confirm());
	reset(); transfer(); auto bytes = SD_MMC.last_data.lock(); assert(bytes);
	(*bytes)[0] = 0; assert(sd_update_confirm()); transfer();
	assert(sd_update_status().error && test_selects == selects);
	reset(); transfer(); bytes = SD_MMC.last_data.lock(); (*bytes)[4090] = 'X';
	assert(sd_update_confirm()); transfer();
	assert(sd_update_status().error && test_selects == selects);
	reset(); transfer(); bytes = SD_MMC.last_data.lock(); bytes->pop_back();
	assert(sd_update_confirm()); transfer(); assert(sd_update_status().error);
	for (int *failure : {&test_begin_error, &test_write_error, &test_end_error, &test_select_error}) {
		reset(); transfer(); assert(sd_update_confirm()); *failure = 1;
		transfer(); assert(sd_update_status().error && !ESP.restarted);
		assert(sd_owner.load() == SdOwner::None);
	}
	reset(); transfer(); assert(sd_update_confirm()); sd_update_poll(++test_now);
	File::short_read = true; transfer();
	assert(sd_update_status().error && !ESP.restarted);
	sd_update_stop();
}
/** @brief Install exactly the source bytes, preserving the card, then reboot after one second across rollover. */
static void success_checks() {
	reset(UINT32_MAX - 2); int selects = test_selects;
	transfer(); assert(sd_update_status().phase == SdUpdatePhase::Ready);
	assert(sd_update_confirm()); transfer();
	assert(sd_update_status().phase == SdUpdatePhase::Success && test_selects == selects + 1);
	assert(test_written == binary() && SD_MMC.files[sd_update_path()] == binary());
	assert(sd_owner.load() == SdOwner::None && !ESP.restarted);
	sd_update_stop(); assert(sd_update_status().active);
	sd_update_poll(test_now + 999); assert(!ESP.restarted);
	sd_update_poll(test_now + 1000); assert(ESP.restarted);
}
#endif

/** @brief Run isolated SD/flash fixtures, never touch a physical card or MCU. @return Success. */
int main() {
#if MOCHI_HAS_SD_AUDIO
	input_checks(); ownership_checks(); confirmation_checks(); failure_checks(); success_checks();
#else
	assert(sd_update_begin(0) && sd_update_status().error);
	assert(!sd_update_confirm());
	sd_update_poll(1000); assert(!ESP.restarted);
	sd_update_stop(); assert(!sd_update_status().active);
#endif
	std::cout << "SD firmware confirmation, compatibility, ownership, chunk bounds and failure checks passed\n";
}

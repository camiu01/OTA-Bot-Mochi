/**
 * @file sd_update.cpp
 * @brief Read-only SD source, checked board/layout, inactive-slot writes and explicit confirmation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "sd_update.h"
#include "ota_image.h"
#include "ota_update.h"
#include "../sound/sound.h"
#include "../bus/sd_owner.h"
#include <Arduino.h>
#if MOCHI_HAS_SD_AUDIO
#include <SD_MMC.h>
#include <esp_ota_ops.h>
#endif

namespace {
SdUpdateStatus state;
uint32_t started = 0, reboot_at = 0;
#if MOCHI_HAS_SD_AUDIO
File file;
OtaImageCheck image;
const esp_partition_t *target = nullptr;
esp_ota_handle_t handle = 0;
bool owned = false, handle_open = false;
uint8_t buffer[4096];

/** @brief Close resources before making the card available to the audio worker. */
void release_card() {
	if (file) { file.close(); }
	if (owned) { SD_MMC.end(); sd_release(SdOwner::Update); owned = false; }
}
/** @brief Fail without selecting the incomplete image or modifying the source card. @param message Recovery detail. */
void fail(const char *message) {
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	release_card();
	state.phase = SdUpdatePhase::Failed; state.error = message;
}
/** @brief Refuse legacy layouts, overlapping slots and the currently running partition. @return Expected OTA v1 pair. */
bool compatible_slots() {
	target = esp_ota_get_next_update_partition(nullptr);
	const auto *running = esp_ota_get_running_partition();
	return target && running && target->size == 0x640000 && running->size == 0x640000 &&
		((target->address == 0x10000 && running->address == 0x650000) ||
		 (target->address == 0x650000 && running->address == 0x10000));
}
/** @brief Mount only after audio has relinquished its files and SDMMC peripheral. */
void open_card() {
	if (!sd_acquire(SdOwner::Update)) { return; }
	owned = true;
	if (!SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA) || !SD_MMC.begin("/sdcard", true, false)) {
		fail("SD unavailable. Power off and check the card."); return;
	}
	file = SD_MMC.open(sd_update_path(), FILE_READ);
	if (!file) { fail("Missing /mochi/update/ release app.bin."); return; }
	state.total = file.size();
	if (state.total < 288 || state.total > target->size) {
		fail("Invalid app size. Use the matching app.bin, not full.bin."); return;
	}
	image = OtaImageCheck{};
	state.phase = SdUpdatePhase::Checking;
}
/** @brief Validate the complete stream before boot selection; ESP-IDF verifies final image integrity. @param now Milliseconds. */
void finish(uint32_t now) {
	if (!image.app_header() || !image.board_found || image.bytes != state.total) {
		fail("Incompatible image. Check the board and OTA layout."); return;
	}
	esp_err_t result = esp_ota_end(handle);
	handle_open = false;
	if (result != ESP_OK) { fail("Image integrity check failed."); return; }
	if (esp_ota_set_boot_partition(target) != ESP_OK) { fail("Cannot select new boot slot."); return; }
	release_card();
	state.phase = SdUpdatePhase::Success; reboot_at = now;
}
/** @brief Check or write at most 4096 bytes per main-loop iteration. @param now Milliseconds. */
void transfer(uint32_t now) {
	bool writing = state.phase == SdUpdatePhase::Writing;
	size_t &position = writing ? state.written : state.checked;
	if (file.size() != state.total) { fail("SD image changed or card removed."); return; }
	size_t count = state.total - position;
	if (count > sizeof(buffer)) { count = sizeof(buffer); }
	if (!count || file.read(buffer, count) != count) { fail("SD read failed. Reopen update mode."); return; }
	image.feed(buffer, count);
	if (!image.app_header()) { fail("Not an ESP32-S3 application image."); return; }
	if (writing) {
		if (!handle_open) {
			if (esp_ota_begin(target, state.total, &handle) != ESP_OK) { fail("Cannot open OTA slot."); return; }
			handle_open = true;
		}
		if (esp_ota_write(handle, buffer, count) != ESP_OK) { fail("Flash write failed."); return; }
	}
	position += count;
	if (position != state.total) { return; }
	if (writing) { finish(now); return; }
	if (!image.board_found) { fail("Wrong board or missing OTA layout marker."); return; }
	if (!file.seek(0)) { fail("Cannot rewind SD image."); return; }
	state.phase = SdUpdatePhase::Ready;
}
#endif
}

const char *sd_update_path() { return "/mochi/update/" MOCHI_OTA_FILENAME; }
const SdUpdateStatus &sd_update_status() { return state; }

bool sd_update_begin(uint32_t now) {
	if (state.active || ota_status().active) { return false; }
	state = SdUpdateStatus{}; state.active = true; started = now;
	sound_init(now);
#if MOCHI_HAS_SD_AUDIO
	state.phase = SdUpdatePhase::Waiting;
	if (!compatible_slots()) { fail("Install the OTA layout using USB first."); }
#else
	state.phase = SdUpdatePhase::Failed; state.error = "SD firmware updates require AMOLED V2.";
#endif
	return true;
}

bool sd_update_confirm() {
#if MOCHI_HAS_SD_AUDIO
	if (state.phase != SdUpdatePhase::Ready) { return false; }
	image = OtaImageCheck{};
	state.phase = SdUpdatePhase::Writing;
	return true;
#else
	return false;
#endif
}

void sd_update_poll(uint32_t now) {
	if (!state.active || state.phase == SdUpdatePhase::Failed) { return; }
	if (state.phase == SdUpdatePhase::Success) {
		if (uint32_t(now - reboot_at) >= 1000) { ESP.restart(); }
		return;
	}
#if MOCHI_HAS_SD_AUDIO
	if (uint32_t(now - started) >= 300000) { fail("Session expired. Reopen update mode."); return; }
	if (state.phase == SdUpdatePhase::Waiting) {
		if (uint32_t(now - started) >= 10000) { fail("Audio owns SD. Restart Mochi and retry."); return; }
		open_card();
	} else if (state.phase == SdUpdatePhase::Checking || state.phase == SdUpdatePhase::Writing) { transfer(now); }
#else
	(void)now;
#endif
}

void sd_update_stop() {
	if (state.phase == SdUpdatePhase::Success) { return; }
#if MOCHI_HAS_SD_AUDIO
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	release_card();
#endif
	state = SdUpdateStatus{};
}

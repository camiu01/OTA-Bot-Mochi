/**
 * @file sd_update.h
 * @brief Manually confirmed, main-task firmware installation from a fixed microSD path.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

enum class SdUpdatePhase : uint8_t { Idle, Waiting, Checking, Ready, Writing, Success, Failed };
struct SdUpdateStatus {
	SdUpdatePhase phase = SdUpdatePhase::Idle;
	bool active = false;
	size_t total = 0, checked = 0, written = 0;
	const char *error = nullptr;
};
/** @brief Enter manual maintenance, silencing audio without altering preferences. @param now Milliseconds. @return Session opened, not installation success. */
bool sd_update_begin(uint32_t now);
/** @brief Confirm a checked image; flash work starts only on the next poll. @return Ready and accepted. */
bool sd_update_confirm();
/** @brief Service one bounded card chunk and delayed reboot, with a five-minute deadline. @param now Milliseconds. */
void sd_update_poll(uint32_t now);
/** @brief Cancel before boot-slot selection; retain the previous firmware and SD files. */
void sd_update_stop();
/** @brief Read main-task progress and recovery information. @return State. */
const SdUpdateStatus &sd_update_status();
/** @brief Fixed, board-specific source filename under /mochi/update/. @return Full card path. */
const char *sd_update_path();

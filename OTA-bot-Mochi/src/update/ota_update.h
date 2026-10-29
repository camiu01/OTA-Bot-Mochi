/**
 * @file ota_update.h
 * @brief Main-task-only, manually enabled local firmware update session.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

/** @brief OTA state with credentials kept in RAM for the current session only. */
struct OtaStatus {
	bool active = false;
	bool faces = false;
	bool lan = false;
	bool receiving = false;
	bool success = false;
	const char *error = nullptr;
	char ssid[33] = {};
	char password[9] = {};
	char address[16] = "192.168.4.1";
	char url[80] = {};
	size_t received = 0;
	size_t total = 0;
};
/** @brief Start a five-minute WPA2 access point. @param now Timestamp. @param progress Optional main-task display refresh. */
void ota_begin(uint32_t now, void (*progress)() = nullptr, bool faces = false);
/** @brief Service HTTP upload and deferred reboot. @param now Timestamp. */
void ota_poll(uint32_t now);
/** @brief Abort an incomplete update and disable Wi-Fi. */
void ota_stop();
/** @brief Read main-task session status. @return Current state. */
const OtaStatus &ota_status();

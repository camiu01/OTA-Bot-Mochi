/**
 * @file session_random.h
 * @brief Fail closed when the radio entropy source required for maintenance secrets is unavailable.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <esp_bt.h>
#include <esp_random.h>
#include <string.h>

/** @brief Require the running BLE controller before generating secrets.
 * @param out Destination buffer. @param size Random byte count. @return Entropy source available. */
inline bool session_random(void *out, size_t size) {
	if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED) { return false; }
	esp_fill_random(out, size);
	return true;
}

/** @brief Compare equal-length tokens without returning at the first mismatched byte.
 * @param provided Request token. @param expected Session token. @return Equal tokens. */
inline bool session_token_equal(const char *provided, const char *expected) {
	if (strlen(provided) != 32 || strlen(expected) != 32) { return false; }
	volatile unsigned difference = 0;
	for (size_t i = 0; i < 32; ++i) { difference |= unsigned(uint8_t(provided[i]) ^ uint8_t(expected[i])); }
	return difference == 0;
}

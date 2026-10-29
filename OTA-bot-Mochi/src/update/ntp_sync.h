/**
 * @file ntp_sync.h
 * @brief One user-requested Wi-Fi/SNTP attempt; main-task clock application only.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <time.h>
/** @brief Bind the same local civil-time setter used by the phone clock. */
void ntp_sync_bind(void (*apply)(const tm &));
/** @brief Validate and start one attempt; credentials are not retained in flash. */
int ntp_sync_start(const char *ssid, const char *password, const char *server, uint8_t zone, uint32_t now);
/** @brief Request time over an already-managed station, without taking ownership of its connection. */
int ntp_sync_connected(const char *server, uint8_t zone, uint32_t now);
/** @brief Service association, asynchronous SNTP completion and the overall deadline. */
void ntp_sync_poll(uint32_t now);
/** @brief Stop SNTP and disconnect only the home-network station, preserving the AP. */
void ntp_sync_stop();
/** @brief Stable status code for localized browser messages; never contains credentials. */
const char *ntp_sync_status();
/** @brief True only during the current user-requested attempt. */
bool ntp_sync_busy();

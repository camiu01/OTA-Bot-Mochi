/**
 * @file network_service.h
 * @brief Main-task ownership of saved Wi-Fi, automatic time, and worker result application.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "network_config.h"
#include "network_weather.h"
#include <Arduino.h>
/** @brief Load saved preferences and start non-blocking association after device initialization. */
void network_begin(uint32_t now);
/** @brief Service bounded policies; suspend new HTTP/NTP jobs during firmware updates. */
void network_poll(uint32_t now, bool allow_jobs = true);
/** @brief Mark the temporary maintenance session and whether its AP must be preserved. */
void network_portal(bool active, bool access_point = false);
/** @brief True when this service owns a connected saved station. */
bool network_connected();
/** @brief Preserve managed station mode after a maintenance AP is closed. */
void network_restore_radio();
/** @brief Current DHCP address, or empty when offline. */
String network_address();
/** @brief Public status without credentials. */
const char *network_status();
/** @brief Start a rate-limited refresh using the saved network; HTTP status result. */
int network_sync_now(uint32_t now);
/** @brief Start a rate-limited weather refresh; HTTP status result. */
int network_weather_now(uint32_t now);
/** @brief Submit one bounded explicit city search; HTTP status result. */
int network_search(const char *query, bool italian);
/** @brief Stable public weather/search status and latest search results. */
const char *network_weather_status();
const char *network_search_status();
const WeatherReply &network_locations();

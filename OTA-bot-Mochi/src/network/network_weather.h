/**
 * @file network_weather.h
 * @brief Fixed-size worker messages; no borrowed strings or UI access across tasks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <ChronosESP32.h>
struct NetworkLocation {
	char name[65] = {}, region[65] = {}, country[3] = {};
	int32_t latitude = 0, longitude = 0;
};
struct WeatherJob {
	uint32_t revision = 0;
	bool search = false, italian = false;
	char query[65] = {};
	int32_t latitude = 0, longitude = 0;
};
struct WeatherReply {
	uint32_t revision = 0;
	bool search = false, ok = false;
	NetworkLocation locations[5] = {};
	uint8_t count = 0;
	Weather days[3] = {};
	char time[6] = {};
};
/** @brief Allocate one low-priority worker and bounded request/result queues. */
bool weather_worker_begin();
/** @brief Queue at most one request without waiting. */
bool weather_worker_submit(const WeatherJob &job);
/** @brief Copy a completed response without waiting, on the main task. */
bool weather_worker_take(WeatherReply &reply);
/** @brief Validate provider JSON and map only requested fields into owned bounded data. */
bool network_weather_parse(const char *json, size_t length, WeatherReply &reply);
/** @brief Convert supported WMO codes to Chronos' existing eight icons; -1 rejects unknown codes. */
int network_weather_icon(int code);

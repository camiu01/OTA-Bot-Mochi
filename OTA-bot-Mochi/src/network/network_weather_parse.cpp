/**
 * @file network_weather_parse.cpp
 * @brief Strict bounded provider fields, independent of HTTP and the rendering task.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "network_weather.h"
#include <cJSON.h>
#include <cmath>
#include <stdio.h>
#include <string.h>

namespace {
/** @brief Reject unusually deep JSON before invoking the platform parser. */
bool bounded_json(const char *json, size_t length) {
	if (!json || !length || length > 8192) { return false; }
	int depth = 0; bool quoted = false, escaped = false;
	for (size_t i = 0; i < length; ++i) {
		char c = json[i];
		if (!c) { return false; }
		if (quoted) {
			if (escaped) { escaped = false; }
			else if (c == '\\') { escaped = true; }
			else if (c == '"') { quoted = false; }
		} else if (c == '"') { quoted = true; }
		else if (c == '{' || c == '[') { if (++depth > 16) { return false; } }
		else if (c == '}' || c == ']') { if (--depth < 0) { return false; } }
	}
	return depth == 0 && !quoted;
}
/** @brief Obtain only finite numeric JSON values in the documented range. */
bool number(const cJSON *item, double low, double high, double &out) {
	if (!cJSON_IsNumber(item) || !std::isfinite(item->valuedouble) ||
		item->valuedouble < low || item->valuedouble > high) { return false; }
	out = item->valuedouble; return true;
}
/** @brief Copy a bounded string, rejecting controls and truncation. */
bool label(const cJSON *item, char *out, size_t capacity, bool optional = false) {
	if (!item && optional) { out[0] = 0; return true; }
	if (!cJSON_IsString(item) || !item->valuestring || !*item->valuestring ||
		strlen(item->valuestring) >= capacity) { return false; }
	for (const char *c = item->valuestring; *c; ++c) { if (uint8_t(*c) < 32 || *c == 127) { return false; } }
	strcpy(out, item->valuestring); return true;
}
/** @brief Read an ISO date and compute the weekday without touching global TZ or the RTC. */
int weekday(const char *text) {
	if (!text || strlen(text) != 10) { return -1; }
	for (int i = 0; i < 10; ++i) {
		if ((i == 4 || i == 7) ? text[i] != '-' : (text[i] < '0' || text[i] > '9')) { return -1; }
	}
	int year = 0, month = 0, day = 0; sscanf(text, "%d-%d-%d", &year, &month, &day);
	static constexpr int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if (year < 2024 || year > 2099 || month < 1 || month > 12 || day < 1 ||
		day > days[month - 1] + (month == 2 && year % 4 == 0)) { return -1; }
	static constexpr int offset[] = {0,3,2,5,0,3,5,1,4,6,2,4};
	if (month < 3) { --year; }
	return (year + year / 4 - year / 100 + year / 400 + offset[month - 1] + day) % 7;
}
/** @brief Read a typed provider object member. */
const cJSON *field(const cJSON *root, const char *name) { return cJSON_GetObjectItemCaseSensitive(root, name); }
/** @brief Validate and retain up to five distinct geocoding choices. */
bool locations(const cJSON *root, WeatherReply &out) {
	const cJSON *items = field(root, "results");
	if (!items) { out.count = 0; return true; }
	if (!cJSON_IsArray(items) || cJSON_GetArraySize(items) > 5) { return false; }
	for (int i = 0; i < cJSON_GetArraySize(items); ++i) {
		const cJSON *item = cJSON_GetArrayItem(items, i);
		NetworkLocation value; double lat = 0, lon = 0;
		if (!label(field(item, "name"), value.name, sizeof(value.name)) ||
			!label(field(item, "admin1"), value.region, sizeof(value.region), true) ||
			!label(field(item, "country_code"), value.country, sizeof(value.country)) ||
			strlen(value.country) != 2 || value.country[0] < 'A' || value.country[0] > 'Z' ||
			value.country[1] < 'A' || value.country[1] > 'Z' ||
			!number(field(item, "latitude"), -90, 90, lat) || !number(field(item, "longitude"), -180, 180, lon)) { return false; }
		value.latitude = int32_t(std::lround(lat * 1000000)); value.longitude = int32_t(std::lround(lon * 1000000));
		out.locations[out.count++] = value;
	}
	return true;
}
/** @brief Parse one day's code, date and high/low, rejecting partial or impossible forecasts. */
bool forecast_day(const cJSON *daily, int i, Weather &day) {
	auto at = [daily, i](const char *name) { return cJSON_GetArrayItem(field(daily, name), i); };
	double code = 0, low = 0, high = 0;
	const cJSON *date = at("time");
	if (!cJSON_IsString(date) || (day.day = weekday(date->valuestring)) < 0 ||
		!number(at("weather_code"), 0, 99, code) || code != std::floor(code) ||
		(day.icon = network_weather_icon(int(code))) < 0 ||
		!number(at("temperature_2m_min"), -90, 70, low) ||
		!number(at("temperature_2m_max"), -90, 70, high) || low > high) { return false; }
	day.low = int(std::lround(low)); day.high = int(std::lround(high)); day.temp = (day.low + day.high) / 2;
	return true;
}
/** @brief Current conditions plus exactly three requested daily forecasts. */
bool forecast(const cJSON *root, WeatherReply &out) {
	const cJSON *daily = field(root, "daily"), *current = field(root, "current");
	for (const char *key : {"time", "weather_code", "temperature_2m_min", "temperature_2m_max"}) {
		if (!cJSON_IsArray(field(daily, key)) || cJSON_GetArraySize(field(daily, key)) != 3) { return false; }
	}
	for (int i = 0; i < 3; ++i) { if (!forecast_day(daily, i, out.days[i])) { return false; } }
	double temperature = 0, code = 0; char time[20] = {};
	if (!number(field(current, "temperature_2m"), -90, 70, temperature) ||
		!number(field(current, "weather_code"), 0, 99, code) || code != std::floor(code) ||
		(out.days[0].icon = network_weather_icon(int(code))) < 0 ||
		!label(field(current, "time"), time, sizeof(time)) || strlen(time) != 16 || time[10] != 'T' ||
		time[11] < '0' || time[11] > '2' || time[12] < '0' || time[12] > '9' || time[13] != ':' ||
		time[14] < '0' || time[14] > '5' || time[15] < '0' || time[15] > '9' ||
		(time[11] == '2' && time[12] > '3')) { return false; }
	memcpy(out.time, time + 11, 5); time[10] = 0;
	if (weekday(time) < 0) { return false; }
	out.days[0].temp = int(std::lround(temperature)); out.count = 3; return true;
}
}
int network_weather_icon(int code) {
	if (code == 0 || code == 1) { return 1; }
	if (code == 2) { return 0; }
	if (code == 3) { return 4; }
	if (code == 45 || code == 48) { return 7; }
	if (code == 51 || code == 53 || code == 55 || code == 56 || code == 57 ||
		code == 61 || code == 63 || code == 65 || code == 66 || code == 67 ||
		code == 80 || code == 81 || code == 82) { return 3; }
	if (code == 71 || code == 73 || code == 75 || code == 77 || code == 85 || code == 86) { return 2; }
	// The existing glyphs lack lightning: use rain, not the wind/extreme glyph.
	if (code == 95 || code == 96 || code == 97 || code == 99) { return 3; }
	return -1;
}
bool network_weather_parse(const char *json, size_t length, WeatherReply &reply) {
	if (!bounded_json(json, length)) { return false; }
	const char *end = nullptr;
	cJSON *root = cJSON_ParseWithLengthOpts(json, length, &end, false);
	if (!root) { return false; }
	bool complete = end && end >= json && end <= json + length;
	if (complete) {
		for (; end < json + length; ++end) {
			if (*end != ' ' && *end != '\n' && *end != '\r' && *end != '\t') { complete = false; break; }
		}
	}
	WeatherReply parsed; parsed.search = reply.search; parsed.revision = reply.revision;
	bool valid = complete && cJSON_IsObject(root) && !cJSON_IsTrue(field(root, "error")) &&
		(parsed.search ? locations(root, parsed) : forecast(root, parsed));
	cJSON_Delete(root);
	if (valid) { parsed.ok = true; reply = parsed; }
	return valid;
}

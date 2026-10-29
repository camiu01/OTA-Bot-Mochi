/**
 * @file network_web.h
 * @brief Authenticated network configuration contract; never return stored passwords.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_web.h"
#include "../network/network_service.h"
#include "ntp_sync.h"
#include <stdlib.h>
#include <limits.h>

namespace network_web {
/** @brief Build public status without copying credentials into JSON. */
inline void state(WebServer &server, int code = 200) {
	const auto &v = network_config();
	String json = String("{\"revision\":") + String(network_config_revision()) +
		",\"enabled\":" + (v.enabled ? "true" : "false") + ",\"ssid\":" + face_web::quoted(v.ssid) +
		",\"password_set\":" + (v.password[0] ? "true" : "false") +
		",\"server\":" + face_web::quoted(v.server) + ",\"zone\":" + String(v.zone) +
		",\"weather\":" + (v.weather ? "true" : "false") + ",\"city\":" + face_web::quoted(v.city) +
		",\"latitude\":" + String(v.latitude) + ",\"longitude\":" + String(v.longitude) +
		",\"status\":" + face_web::quoted(network_status()) + ",\"ip\":" + face_web::quoted(network_address().c_str()) +
		",\"ntp\":" + face_web::quoted(ntp_sync_status()) +
		",\"weather_status\":" + face_web::quoted(network_weather_status()) + "}";
	server.send(code, "application/json", json);
}
/** @brief Require canonical bounded integers, including coordinates. */
inline bool integer(const String &raw, int32_t low, int32_t high, int32_t &out) {
	if (!raw.length() || raw.length() > 11 || raw.length() != strlen(raw.c_str())) { return false; }
	char *end = nullptr; long long value = strtoll(raw.c_str(), &end, 10);
	if (!end || *end || value < low || value > high || !(String(int32_t(value)) == raw)) { return false; }
	out = int32_t(value); return true;
}
/** @brief Reject embedded NUL, missing fields and truncation before copying. */
inline bool copy(WebServer &s, const char *key, char *out, size_t capacity) {
	String value = s.arg(key);
	if (!s.hasArg(key) || value.length() >= capacity || value.length() != strlen(value.c_str())) { return false; }
	strcpy(out, value.c_str()); return true;
}
/** @brief Exact form media type; expected key presence also excludes same-count extra fields. */
inline bool form(WebServer &s, int count) {
	return s.header("Content-Type") == "application/x-www-form-urlencoded" && s.args() == count;
}
/** @brief Validate the complete candidate before writing NVS or requesting reconnection. */
inline bool candidate(WebServer &s, NetworkConfig &next) {
	if (!form(s, 10)) { return false; }
	int32_t enabled = 0, weather = 0, zone = 0;
	for (const char *key : {"enabled", "weather", "zone", "latitude", "longitude", "password_mode"}) {
		if (!s.hasArg(key)) { return false; }
	}
	if (!integer(s.arg("enabled"), 0, 1, enabled) || !integer(s.arg("weather"), 0, 1, weather) ||
		!integer(s.arg("zone"), 0, 1, zone) || !integer(s.arg("latitude"), -90000000, 90000000, next.latitude) ||
		!integer(s.arg("longitude"), -180000000, 180000000, next.longitude) ||
		!copy(s, "ssid", next.ssid, sizeof(next.ssid)) || !copy(s, "server", next.server, sizeof(next.server)) ||
		!copy(s, "city", next.city, sizeof(next.city)) || !s.hasArg("password")) { return false; }
	next.enabled = enabled; next.weather = weather; next.zone = uint8_t(zone);
	String mode = s.arg("password_mode");
	if (mode == "keep") {
		if (!network_config_exists() || strcmp(next.ssid, network_config().ssid) || s.arg("password").length()) { return false; }
		strcpy(next.password, network_config().password);
	} else if (mode == "open") {
		if (s.arg("password").length()) { return false; }
		next.password[0] = 0;
	} else if (!(mode == "replace") || !copy(s, "password", next.password, sizeof(next.password)) || !next.password[0]) { return false; }
	return network_config_valid(next);
}
/** @brief Preserve old connection and configuration on flash failure; expose stale edits explicitly. */
inline void save(WebServer &s) {
	if (!(s.header("X-Mochi-Network-Revision") == String(network_config_revision()))) {
		s.send(409, "application/json", "{\"error\":\"Reload network settings\"}"); return;
	}
	NetworkConfig next;
	if (!candidate(s, next)) { s.send(400, "application/json", "{\"error\":\"Invalid network settings\"}"); return; }
	if (!network_config_save(next)) { s.send(507, "application/json", "{\"error\":\"Storage failed; previous network retained\"}"); return; }
	memset(next.password, 0, sizeof(next.password)); state(s);
}
/** @brief Explicitly replace saved secrets with disabled defaults, without erasing unrelated preferences. */
inline void forget(WebServer &s) {
	if (!form(s, 1) || !s.hasArg("revision")) { s.send(400, "application/json", "{\"error\":\"Invalid form\"}"); return; }
	if (!(s.arg("revision") == String(network_config_revision()))) { s.send(409, "application/json", "{\"error\":\"Reload network settings\"}"); return; }
	if (!network_config_save(NetworkConfig{})) { s.send(507, "application/json", "{\"error\":\"Storage failed\"}"); return; }
	state(s);
}
/** @brief Publish only validated location names, regions, countries and coordinates. */
inline void locations(WebServer &s, int code = 200) {
	const auto &reply = network_locations();
	String json = String("{\"status\":") + face_web::quoted(network_search_status()) + ",\"locations\":[";
	for (uint8_t i = 0; i < reply.count; ++i) {
		const auto &v = reply.locations[i]; if (i) { json += ','; }
		json += String("{\"name\":") + face_web::quoted(v.name) + ",\"region\":" + face_web::quoted(v.region) +
			",\"country\":" + face_web::quoted(v.country) + ",\"latitude\":" + String(v.latitude) +
			",\"longitude\":" + String(v.longitude) + "}";
	}
	s.send(code, "application/json", json + "]}");
}
/** @brief Search only after an explicit authenticated form submission. */
inline void search(WebServer &s) {
	char query[65];
	if (!form(s, 1) || !copy(s, "query", query, sizeof(query))) { s.send(400, "application/json", "{\"error\":\"Invalid query\"}"); return; }
	locations(s, network_search(query, settings_get().language != 0));
}
}

/**
 * @file wifi_scan.h
 * @brief Main-task asynchronous scan, bounded public results and explicit cleanup.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <WiFi.h>
#include "network_web.h"
#include <esp_wifi.h>

namespace wifi_scan {
inline bool running = false, attempted = false;
inline uint32_t started = 0;
/** @brief Cancel only our scan and free driver-owned results before mode changes. */
inline void stop() {
	if (running) { esp_wifi_scan_stop(); }
	WiFi.scanDelete(); running = false; attempted = false;
}
/** @brief Start on explicit authenticated POST; avoid scanning while station association is in progress. */
inline void start(WebServer &s) {
	if (!network_web::form(s, 1) || !(s.arg("action") == "scan")) {
		s.send(400, "application/json", "{\"error\":\"Invalid form\"}"); return;
	}
	if (running || (network_config().enabled && !network_connected()) || ntp_sync_busy()) {
		s.send(409, "application/json", "{\"error\":\"Wi-Fi busy\"}"); return;
	}
	if (attempted && uint32_t(millis() - started) < 15000) {
		s.send(429, "application/json", "{\"error\":\"Wait before scanning again\"}"); return;
	}
	WiFi.scanDelete();
	int result = WiFi.scanNetworks(true, false, false, 120);
	started = millis(); attempted = true; running = result == WIFI_SCAN_RUNNING;
	s.send(running || result >= 0 ? 202 : 503, "application/json", "{\"status\":\"scanning\"}");
}
/** @brief Deadline cleanup also runs without a polling browser. */
inline void poll(uint32_t now) {
	if (running && uint32_t(now - started) >= 15000) { esp_wifi_scan_stop(); WiFi.scanDelete(); running = false; }
}
/** @brief Publish at most 24 distinct valid SSIDs, escaped as JSON, never HTML. */
inline void results(WebServer &s) {
	int count = attempted ? WiFi.scanComplete() : WIFI_SCAN_FAILED;
	if (count != WIFI_SCAN_RUNNING) { running = false; }
	String json = String("{\"status\":\"") + (count == WIFI_SCAN_RUNNING ? "scanning" : count < 0 ? "failed" : "ready") + "\",\"networks\":[";
	String seen[24]; size_t used = 0;
	for (int i = 0; i < count && i < 128 && used < 24; ++i) {
		String name = WiFi.SSID(i); NetworkConfig candidate;
		if (!name.length() || name.length() > 32 || name.length() != strlen(name.c_str())) { continue; }
		strcpy(candidate.ssid, name.c_str());
		if (!network_config_valid(candidate)) { continue; }
		bool duplicate = false;
		for (size_t j = 0; j < used; ++j) { if (seen[j] == name) { duplicate = true; } }
		if (duplicate) { continue; }
		if (used) { json += ','; } seen[used++] = name;
		json += face_web::quoted(name.c_str());
	}
	s.send(200, "application/json", json + "]}");
}
}

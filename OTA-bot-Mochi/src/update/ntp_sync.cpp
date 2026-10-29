/**
 * @file ntp_sync.cpp
 * @brief Bounded asynchronous network time; no boot connection or periodic resync.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ntp_sync.h"
#include <WiFi.h>
#include <esp_sntp.h>
#include <atomic>
#include <string.h>
#include <sys/time.h>

namespace {
void (*apply_clock)(const tm &) = nullptr;
std::atomic<uint32_t> received{0};
std::atomic<bool> accepting{false};
const char *status = "idle";
bool busy = false, querying = false, station = false;
uint32_t started = 0;
char host[64] = {};
uint8_t local_zone = 0;
/** @brief Check bounded strings before touching radio state. */
bool valid(const char *ssid, const char *password, const char *server, uint8_t zone) {
	if (!ssid || !password || !server || !strlen(ssid) || strlen(ssid) > 32 || zone > 1) { return false; }
	size_t p = strlen(password), n = strlen(server);
	if ((p && p < 8) || p > 64 || !n || n > 63 || server[0] == '.' || server[n - 1] == '.') { return false; }
	for (const char *c = ssid; *c; ++c) { if (uint8_t(*c) < 32 || *c == 127) { return false; } }
	for (size_t i = 0; i < p; ++i) {
		unsigned char c = password[i];
		if (c < 32 || c == 127 || (p == 64 && !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))) { return false; }
	}
	for (const char *c = server; *c; ++c) {
		if (!( (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == '-' || *c == '.')) { return false; }
	}
	return true;
}
/** @brief Stop the background client before releasing its hostname or station interface. */
void disconnect() {
	if (querying) { esp_sntp_stop(); querying = false; }
	accepting.store(false);
	received.store(0);
	if (station) { WiFi.disconnect(false, true); WiFi.mode(WIFI_AP); station = false; }
	busy = false;
}
/** @brief EU daylight saving starts/ends on the last Sunday at 01:00 UTC. */
bool summer(const tm &utc) {
	if (utc.tm_mon > 2 && utc.tm_mon < 9) { return true; }
	if (utc.tm_mon != 2 && utc.tm_mon != 9) { return false; }
	int sunday = 31 - (utc.tm_wday + 31 - utc.tm_mday) % 7;
	bool after = utc.tm_mday > sunday || (utc.tm_mday == sunday && utc.tm_hour >= 1);
	return utc.tm_mon == 2 ? after : !after;
}
/** @brief Use the platform's reentrant UTC conversion, including the native Windows harness. */
void utc_time(time_t epoch, tm &out) {
#if defined(_WIN32)
	gmtime_s(&out, &epoch);
#else
	gmtime_r(&epoch, &out);
#endif
}
/** @brief Apply validated civil time only on the main task, never changing global TZ. */
void finish() {
	time_t epoch = time_t(received.exchange(0));
	if (!epoch) { return; }
	disconnect();
	if (epoch < 1704067200LL || epoch > 4102444799LL) {
		status = "invalid_time"; return;
	}
	tm local{}; utc_time(epoch, local);
	if (local_zone) { epoch += summer(local) ? 7200 : 3600; utc_time(epoch, local); }
	apply_clock(local); status = "synced";
}
}
/**
 * @brief Override IDF's documented weak clock writer: SNTP never writes the shared
 * clock on its TCP/IP task. This module is the application's sole SNTP owner.
 */
extern "C" void sntp_sync_time(timeval *tv) {
	int64_t epoch = tv->tv_sec;
	if (accepting.load()) { received.store(epoch >= 1704067200LL && epoch <= 4102444799LL ? uint32_t(epoch) : 1); }
}
void ntp_sync_bind(void (*apply)(const tm &)) { apply_clock = apply; }
int ntp_sync_connected(const char *server, uint8_t zone, uint32_t now) {
	if (busy) { return 409; }
	if (!apply_clock || WiFi.status() != WL_CONNECTED) { return 503; }
	if (!valid("managed", "", server, zone)) { return 400; }
	received.store(0); strncpy(host, server, sizeof(host) - 1);
	local_zone = zone; started = now; status = "connecting"; busy = true; station = false;
	return 202;
}
int ntp_sync_start(const char *ssid, const char *password, const char *server, uint8_t zone, uint32_t now) {
	if (busy) { return 409; }
	if (!apply_clock) { return 503; }
	if (!valid(ssid, password, server, zone)) { return 400; }
	received.store(0); strncpy(host, server, sizeof(host) - 1);
	local_zone = zone; started = now; status = "connecting"; busy = station = true;
	WiFi.persistent(false);
	WiFi.setAutoReconnect(false);
	if (!WiFi.mode(WIFI_AP_STA)) { disconnect(); status = "wifi_failed"; return 503; }
	WiFi.begin(ssid, password);
	return 202;
}
void ntp_sync_poll(uint32_t now) {
	if (!busy) { return; }
	if (received.load()) { finish(); return; }
	if (uint32_t(now - started) >= 25000) { disconnect(); status = "timeout"; return; }
	if (!querying && WiFi.status() == WL_CONNECTED) {
		status = "querying"; querying = true; accepting.store(true);
		esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
		esp_sntp_setservername(0, host);
		esp_sntp_init();
	}
}
void ntp_sync_stop() {
	if (querying) { esp_sntp_stop(); querying = false; }
	accepting.store(false);
	if (received.load() && apply_clock) { finish(); }
	if (busy) { status = "cancelled"; }
	disconnect();
}
const char *ntp_sync_status() { return status; }
bool ntp_sync_busy() { return busy; }

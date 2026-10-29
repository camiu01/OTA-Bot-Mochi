/**
 * @file network_service.cpp
 * @brief Reconnect and refresh without blocking rendering, or changing hardware/OTA layout.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "network_service.h"
#include "../update/ntp_sync.h"
#include "../ui/ui_state.h"
#include <WiFi.h>
#include <string.h>

namespace {
bool initialized = false, portal_ap = false;
bool online = false, associating = false, worker_ready = false, worker_busy = false;
bool ntp_tried = false, weather_tried = false, changed = false;
uint32_t seen_revision = 0, changed_at = 0, attempt_at = 0, ntp_at = 0, weather_at = 0;
const char *status = "unconfigured", *weather_status = "idle", *search_status = "idle";
WeatherReply locations;
constexpr uint32_t CONNECT_MS = 20000, RECONNECT_MS = 60000;
/** @brief Start one station attempt, preserving only an explicitly opened maintenance AP. */
void connect(uint32_t now) {
	attempt_at = now; associating = false; online = false;
	WiFi.persistent(false); WiFi.setAutoReconnect(false);
	if (!WiFi.mode(portal_ap ? WIFI_AP_STA : WIFI_STA)) { status = "wifi_failed"; return; }
	const auto &config = network_config();
	WiFi.begin(config.ssid, config.password);
	associating = true; status = "connecting";
}
/** @brief Apply saved changes after allowing the HTTP save response to leave the socket. */
void configure(uint32_t now) {
	ntp_sync_stop(); WiFi.disconnect(false, true);
	ui_network_weather_clear();
	seen_revision = network_config_revision(); changed = false;
	online = associating = ntp_tried = weather_tried = false;
	weather_status = search_status = "idle"; locations = WeatherReply{};
	if (network_config().enabled) { connect(now); }
	else {
		status = network_config_exists() ? "disabled" : "unconfigured";
		if (!portal_ap) { WiFi.mode(WIFI_OFF); }
	}
}
/** @brief Ignore old-generation replies after any configuration change. */
void completed(uint32_t now) {
	WeatherReply reply;
	if (!weather_worker_take(reply)) { return; }
	worker_busy = false;
	if (reply.revision != network_config_revision()) { return; }
	if (reply.search) {
		locations = reply; search_status = reply.ok ? (reply.count ? "ready" : "empty") : "failed";
	} else {
		weather_status = reply.ok ? "ready" : "failed";
		if (reply.ok && network_config().weather) {
			ui_network_weather(reply.days, network_config().city, reply.time, now);
		}
	}
}
/** @brief A single queue slot and generation number bound worker ownership. */
bool submit(WeatherJob job) {
	job.revision = network_config_revision();
	if (!worker_ready || worker_busy || !weather_worker_submit(job)) { return false; }
	worker_busy = true; return true;
}
/** @brief Dispatch one automatic time attempt, then weather when its interval expires. */
void automatic(uint32_t now) {
	const auto &config = network_config();
	bool synced = strcmp(ntp_sync_status(), "synced") == 0;
	if (!ntp_sync_busy() && (!ntp_tried || uint32_t(now - ntp_at) >= (synced ? 21600000u : 300000u))) {
		network_sync_now(now);
	}
	if (config.weather && !ntp_sync_busy() && !worker_busy &&
		(!weather_tried || uint32_t(now - weather_at) >= (strcmp(weather_status, "ready") == 0 ? 900000u : 300000u))) {
		network_weather_now(now);
	}
}
}
void network_begin(uint32_t now) {
	network_config_init();
	initialized = true; portal_ap = false;
	worker_ready = weather_worker_begin(); worker_busy = false;
	configure(now);
}
void network_poll(uint32_t now, bool allow_jobs) {
	if (!initialized) { return; }
	completed(now);
	if (seen_revision != network_config_revision()) {
		if (!changed) { changed = true; changed_at = now; }
		if (uint32_t(now - changed_at) >= 500) { configure(now); }
		return;
	}
	ntp_sync_poll(now);
	if (!network_config().enabled) { return; }
	if (WiFi.status() == WL_CONNECTED) {
		if (!online) { online = true; associating = false; ntp_tried = false; status = "connected"; }
		if (allow_jobs) { automatic(now); }
		return;
	}
	if (online) { online = false; associating = false; attempt_at = now; status = "offline"; ntp_sync_stop(); }
	if (associating && uint32_t(now - attempt_at) >= CONNECT_MS) {
		associating = false; attempt_at = now; status = "offline";
		WiFi.disconnect(false, true);
	}
	if (!associating && uint32_t(now - attempt_at) >= RECONNECT_MS) { connect(now); }
}
void network_portal(bool active, bool access_point) {
	portal_ap = active && access_point;
}
bool network_connected() {
	return initialized && seen_revision == network_config_revision() && network_config().enabled && online && WiFi.status() == WL_CONNECTED;
}
void network_restore_radio() {
	if (initialized && network_config().enabled) { WiFi.mode(WIFI_STA); }
	else { WiFi.mode(WIFI_OFF); }
}
String network_address() { return network_connected() ? WiFi.localIP().toString() : String(""); }
const char *network_status() { return initialized && seen_revision != network_config_revision() ? "updating" : status; }
int network_sync_now(uint32_t now) {
	if (!network_connected()) { return 409; }
	if (ntp_sync_busy()) { return 409; }
	if (ntp_tried && uint32_t(now - ntp_at) < 30000) { return 429; }
	ntp_at = now; ntp_tried = true;
	return ntp_sync_connected(network_config().server, network_config().zone, now);
}
int network_weather_now(uint32_t now) {
	if (!network_connected() || !network_config().weather || ntp_sync_busy() || worker_busy) { return 409; }
	if (weather_tried && uint32_t(now - weather_at) < 30000) { return 429; }
	WeatherJob job; job.latitude = network_config().latitude; job.longitude = network_config().longitude;
	weather_at = now; weather_tried = true;
	if (!submit(job)) { weather_status = "unavailable"; return 503; }
	weather_status = "loading"; return 202;
}
int network_search(const char *query, bool italian) {
	if (!query || strlen(query) < 2 || strlen(query) > 64) { return 400; }
	for (const char *c = query; *c; ++c) { if (uint8_t(*c) < 32 || *c == 127) { return 400; } }
	if (!network_connected() || ntp_sync_busy() || worker_busy) { return 409; }
	WeatherJob job; job.search = true; job.italian = italian; strcpy(job.query, query);
	if (!submit(job)) { search_status = "unavailable"; return 503; }
	locations = WeatherReply{}; search_status = "loading"; return 202;
}
const char *network_weather_status() { return weather_status; }
const char *network_search_status() { return search_status; }
const WeatherReply &network_locations() { return locations; }

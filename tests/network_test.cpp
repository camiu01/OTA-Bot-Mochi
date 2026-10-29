/**
 * @file network_test.cpp
 * @brief Saved-network integrity, reconnect/refresh policy and real local-route authorization.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/network/network_service.h"
#include "../OTA-bot-Mochi/src/update/network_web.h"
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/ui/ui_state.h"
#include <network_weather.h>
#include <esp_sntp.h>
#include <Preferences.h>
#include <cassert>
#include <iostream>
#include <string>

static int clock_updates = 0;
static void clock_set(const tm &) { ++clock_updates; }
static void tick(uint32_t now, bool jobs = true) { test_now = now; network_poll(now, jobs); }
static void time_reply(uint32_t now) {
	tick(now); timeval time{}; time.tv_sec = 1784116800; sntp_sync_time(&time); tick(now + 1);
}
static NetworkConfig fixture() {
	NetworkConfig value; value.enabled = true;
	strcpy(value.ssid, "fixture-network"); strcpy(value.password, "fixture-secret");
	return value;
}
static void persistence() {
	assert(!network_config_init() && !network_config_exists() && !network_config().enabled);
	NetworkConfig value = fixture(); auto profile = test_profile;
	assert(network_config_save(value) && test_profile == profile);
	uint32_t revision = network_config_revision(); unsigned writes = test_flash_writes;
	test_save_fails = true; strcpy(value.ssid, "new-fixture");
	assert(!network_config_save(value) && network_config_revision() == revision && test_flash_writes == writes);
	assert(strcmp(network_config().ssid, "fixture-network") == 0);
	test_save_fails = false; assert(network_config_init() && strcmp(network_config().password, "fixture-secret") == 0);
	auto record = test_nvs["mochi-net/config"];
	test_nvs["mochi-net/config"].back() ^= 1; assert(!network_config_init() && !network_config().enabled);
	test_nvs["mochi-net/config"] = record; test_nvs["mochi-net/config"].pop_back(); assert(!network_config_init());
	test_nvs["mochi-net/config"] = record; assert(network_config_init());
	for (const char *password : {"short", "bad\npassword", "123456789012345678901234567890123456789012345678901234567890123g"}) {
		value = fixture(); strcpy(value.password, password); assert(!network_config_valid(value));
	}
	value = fixture(); memset(value.ssid, 'x', sizeof(value.ssid)); assert(!network_config_valid(value));
	value = fixture(); strcpy(value.ssid, "caff\xc3\xa8"); assert(network_config_valid(value));
	for (const char *invalid : {"\xc3", "\xc0\xaf", "\xed\xa0\x80", "\xf4\x90\x80\x80"}) {
		value = fixture(); strcpy(value.ssid, invalid); assert(!network_config_valid(value));
	}
	value = fixture(); value.weather = true; assert(!network_config_valid(value));
	value = fixture(); value.latitude = 90000001; assert(!network_config_valid(value));
	value = fixture(); strcpy(value.server, "https://pool.ntp.org"); assert(!network_config_valid(value));
	test_nvs.erase("mochi-net/config"); assert(!network_config_init());
}
static void policy() {
	network_begin(UINT32_MAX - 1000); assert(!ota_status().active && WiFi.mode_value == WIFI_OFF);
	assert(WiFi.station_begins == 0);
	network_portal(true, true); WiFi.mode(WIFI_AP);
	assert(network_config_save(fixture()));
	tick(UINT32_MAX - 500); tick(0); assert(WiFi.mode_value == WIFI_AP_STA && WiFi.station_begins == 1);
	tick(19999); assert(strcmp(network_status(), "connecting") == 0);
	tick(20000); assert(strcmp(network_status(), "offline") == 0 && !ota_status().active);
	tick(79999); assert(WiFi.station_begins == 1); tick(80000); assert(WiFi.station_begins == 2);
	WiFi.connection = WL_CONNECTED; tick(80001); assert(network_connected() && ntp_sync_busy());
	int disconnects = WiFi.station_disconnects; time_reply(80002);
	assert(clock_updates == 1 && !ntp_sync_busy() && WiFi.station_disconnects == disconnects);
	assert(network_sync_now(80005) == 429 && network_search("x", false) == 400);
	assert(network_search("Berlin", true) == 202 && test_weather_job.search && test_weather_job.italian);
	assert(network_search("Berlin", true) == 409);
	test_weather_reply = WeatherReply{}; test_weather_reply.search = test_weather_reply.ok = true;
	test_weather_reply.revision = test_weather_job.revision; test_weather_reply.count = 1;
	strcpy(test_weather_reply.locations[0].name, "Berlin"); test_weather_ready = true;
	tick(80004); assert(strcmp(network_search_status(), "ready") == 0 && network_locations().count == 1);
	NetworkConfig value = network_config(); value.weather = true; strcpy(value.city, "Berlin");
	value.latitude = 52524370; value.longitude = 13410530; assert(network_config_save(value));
	tick(80005); tick(80505); WiFi.connection = WL_CONNECTED; tick(80506); time_reply(80507);
	assert(test_weather_pending && !test_weather_job.search && test_weather_job.latitude == value.latitude);
	test_weather_reply = WeatherReply{}; test_weather_reply.ok = true; test_weather_reply.revision = test_weather_job.revision;
	test_weather_reply.days[0].temp = 21; strcpy(test_weather_reply.time, "14:00"); test_weather_ready = true;
	tick(80509); auto snapshot = ui_snapshot();
	assert(snapshot.weather_internet && snapshot.weather[0].temp == 21 && snapshot.city == "Berlin");
	unsigned jobs = test_weather_jobs;
	tick(80509 + 899998); assert(test_weather_jobs == jobs);
	tick(80509 + 900000); assert(test_weather_jobs == jobs + 1);
	test_weather_reply.ok = false; test_weather_ready = true; tick(980510);
	assert(ui_snapshot().weather[0].temp == 21);
	tick(980511); assert(test_weather_jobs == jobs + 1);
	network_portal(false); network_restore_radio(); WiFi.connection = 0; tick(980512);
	assert(!network_connected() && WiFi.mode_value == WIFI_STA);
}
static void invoke(WebServer &server, const char *path) {
	assert(server.routes.count(path)); server.routes.at(path)();
}
static void form(WebServer &server) {
	const auto &v = network_config();
	server.headers["Content-Type"] = "application/x-www-form-urlencoded";
	server.headers["X-Mochi-Network-Revision"] = String(network_config_revision());
	server.arguments = {{"enabled","1"},{"ssid",v.ssid},{"password",""},{"password_mode","keep"},
		{"server",v.server},{"zone","1"},{"weather","1"},{"city",v.city},
		{"latitude",String(v.latitude)},{"longitude",String(v.longitude)}};
}
static void routes() {
	WiFi.connection = WL_CONNECTED; tick(980513, false);
	assert(network_search("Berlin", false) == 202);
	auto old_revision = test_weather_job.revision;
	assert(network_config_save(network_config()));
	test_weather_reply = WeatherReply{}; test_weather_reply.ok = test_weather_reply.search = true;
	test_weather_reply.revision = old_revision; test_weather_reply.count = 1; test_weather_ready = true;
	tick(980514, false); assert(network_locations().count == 0);
	tick(981014, false); WiFi.connection = WL_CONNECTED; tick(981015, false);
	ota_begin(test_now, nullptr, true); assert(ota_status().lan);
	auto &s = *WebServer::instance; s.host = "192.168.1.42"; s.local_address = WiFi.ip; s.arguments.clear(); s.get();
	std::string url = ota_status().url, token = url.substr(url.find('#') + 1);
	assert(s.code == 200 && token.size() == 32 && std::string(s.body.c_str()).find(token) == std::string::npos);
	assert(!ota_status().password[0]);
	s.headers["X-Mochi-Token"] = token;
	for (const char *route : {"GET /network", "GET /locations"}) { invoke(s, route); assert(s.code == 200); }
	assert(std::string(s.body.c_str()).find("fixture-secret") == std::string::npos);
	form(s); s.arguments["zone"] = "2"; invoke(s, "POST /network"); assert(s.code == 400);
	form(s); s.arguments["latitude"] = "nan"; invoke(s, "POST /network"); assert(s.code == 400);
	form(s); s.arguments["ssid"] = "changed-network"; invoke(s, "POST /network"); assert(s.code == 400);
	form(s); s.arguments["password_mode"] = "open"; s.arguments["password"] = "not-empty";
	invoke(s, "POST /network"); assert(s.code == 400);
	form(s); test_save_fails = true; invoke(s, "POST /network"); assert(s.code == 507);
	assert(strcmp(network_config().password, "fixture-secret") == 0); test_save_fails = false;
	form(s); invoke(s, "POST /network"); assert(s.code == 200);
	invoke(s, "POST /network"); assert(s.code == 409);
	s.headers["X-Mochi-Token"] = "bad";
	for (const char *route : {"GET /network", "POST /network", "POST /network/forget", "GET /locations", "POST /locations", "POST /weather/refresh"}) {
		invoke(s, route); assert(s.code == 403);
	}
	s.headers["X-Mochi-Token"] = token; s.host = "192.168.4.1"; s.get(); assert(s.code == 403);
	s.host = "192.168.1.42"; s.local_address = IPAddress(192,168,4,1); s.get(); assert(s.code == 403);
	s.local_address = WiFi.ip; s.arguments = {{"revision",String(network_config_revision())}};
	s.headers["Content-Type"] = "application/x-www-form-urlencoded";
	invoke(s, "POST /network/forget"); assert(s.code == 200 && !network_config().enabled && !network_config().password[0]);
	ota_stop(); uint32_t closed = test_now; tick(closed + 1); tick(closed + 501);
	assert(WiFi.mode_value == WIFI_OFF && !ui_snapshot().weather_internet);
	assert(network_config_init() && !network_config().enabled);
}
int main() {
	assert(ui_state_init() && settings_init()); ntp_sync_bind(clock_set);
	persistence(); policy(); routes();
	std::cout << "Saved network integrity, rollover/reconnect, shared NTP, weather cadence/cache, LAN QR/token/Host and atomic web saves passed\n";
}

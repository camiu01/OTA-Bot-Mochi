/**
 * @file settings_web_test.cpp
 * @brief Real registered settings/NTP handlers, atomic edits and maintenance boundaries.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/update/settings_web.h"
#include "../OTA-bot-Mochi/src/update/ntp_sync.h"
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <esp_sntp.h>
#include <cassert>
#include <iostream>
#include <string>

static void call(WebServer &s, const char *path) { assert(s.routes.count(path)); s.routes.at(path)(); }
static WebServer &session() {
	ota_stop(); test_now = 100; ota_begin(test_now, nullptr, true);
	auto &s = *WebServer::instance; s.host = "192.168.4.1"; s.arguments.clear(); s.get();
	std::string html = s.body.c_str(), marker = "const token='";
	auto at = html.find(marker); assert(at != std::string::npos);
	s.headers["X-Mochi-Token"] = html.substr(at + marker.size(), 32);
	s.headers["Content-Type"] = "application/x-www-form-urlencoded"; return s;
}
static void form(WebServer &s) {
	s.arguments.clear();
	for (const auto &f : settings_web::fields) { s.arguments[f.key] = String(settings_web::value(f, settings_get())); }
	s.headers["X-Mochi-Settings-Revision"] = settings_web::revision();
}
static void edits(WebServer &s) {
	call(s, "GET /settings"); assert(s.code == 200);
	assert(std::string(s.body.c_str()).find("\"auto_rotate\"") != std::string::npos);
	assert(std::string(s.body.c_str()).find("\"phone_ios\"") != std::string::npos);
	call(s, "GET /settings-ui"); assert(s.code == 200 && std::string(s.body.c_str()).find("netForm") != std::string::npos);
	form(s); s.arguments["brightness"] = "70"; s.arguments["auto_rotate"] = "0";
	call(s, "POST /settings"); assert(s.code == 200 && settings_get().brightness == 70 && !imu_orientation().enabled);
	form(s); s.arguments["phone_ios"] = "1";
	call(s, "POST /settings"); assert(s.code == 200 && settings_get().phone_ios);
	call(s, "POST /settings"); assert(s.code == 409);
	form(s); auto original = settings_get(); unsigned writes = test_flash_writes;
	s.arguments["brightness"] = "90"; s.arguments["sound_level"] = "31";
	call(s, "POST /settings"); assert(s.code == 400 && settings_get().brightness == original.brightness && test_flash_writes == writes);
	for (const char *bad : {"", "-1", "5.0", "005", "256", "nan", " 70"}) {
		form(s); s.arguments["brightness"] = bad; call(s, "POST /settings"); assert(s.code == 400);
	}
	form(s); s.arguments.erase("dim"); call(s, "POST /settings"); assert(s.code == 400);
	form(s); s.arguments["unexpected"] = "1"; call(s, "POST /settings"); assert(s.code == 400);
	FaceFilter mask; mask.set(HAPPY, false); assert(settings_faces_update(mask));
	form(s); s.arguments["mood"] = "1"; call(s, "POST /settings"); assert(s.code == 400 && settings_get().mood != HAPPY);
	form(s); s.arguments["brightness"] = "50"; test_save_fails = true; call(s, "POST /settings");
	assert(s.code == 507 && settings_dirty() && settings_get().brightness == 50);
	test_save_fails = false; s.headers["X-Mochi-Settings-Revision"] = settings_web::revision();
	call(s, "POST /settings"); assert(s.code == 200 && !settings_dirty());
	s.arguments = {{"action", "calibrate"}}; call(s, "POST /settings/action");
	assert(s.code == 200 && imu_orientation().status == ImuCalibration::Failed);
	s.arguments = {{"action", "test"}}; call(s, "POST /settings/action"); assert(s.code == 200);
	s.arguments = {{"action", "invalid"}}; call(s, "POST /settings/action"); assert(s.code == 400);
}
static void clock_set(const tm &) {}
static void clock_and_auth(WebServer &s) {
	assert(WiFi.station_begins == 0); ntp_sync_bind(clock_set);
	s.arguments = {{"ssid", "fixture-net"}, {"password", "fixture-pass"}, {"server", "pool.ntp.org"}, {"zone", "1"}};
	call(s, "POST /ntp"); assert(s.code == 202 && ntp_sync_busy());
	call(s, "POST /ntp"); assert(s.code == 409);
	call(s, "GET /ntp"); assert(s.code == 200 && std::string(s.body.c_str()).find("fixture") == std::string::npos);
	String token = s.headers["X-Mochi-Token"]; s.headers["X-Mochi-Token"] = "bad";
	for (const char *path : {"GET /settings", "POST /settings", "POST /settings/action", "GET /ntp", "POST /ntp"}) {
		call(s, path); assert(s.code == 403);
	}
	s.headers["X-Mochi-Token"] = token; s.host = "other.example";
	call(s, "GET /settings-ui"); assert(s.code == 403);
	s.host = "192.168.4.1"; s.arguments = {{"action", "firmware"}};
	call(s, "POST /settings/action"); assert(s.code == 200 && !ota_status().faces && !ntp_sync_busy());
	call(s, "GET /settings"); assert(s.code == 403);
	call(s, "POST /ntp"); assert(s.code == 403);
	ota_stop(); session(); test_now += 300000;
	call(s, "POST /settings"); assert(s.code == 403);
	call(s, "GET /settings-ui"); assert(s.code == 403);
	ota_poll(test_now); assert(!ota_status().active && WiFi.mode_value == WIFI_OFF);
}
int main() {
	assert(settings_init()); eyes_init(nullptr); assert(settings_save());
	auto &s = session(); edits(s); clock_and_auth(s);
	// Generate an offline schema fixture for browser testing, with no session credentials.
	if (std::getenv("MOCHI_WEB_FIXTURE")) {
		settings_web::state(s);
		FILE *f = fopen(std::getenv("MOCHI_WEB_FIXTURE"), "wb"); assert(f);
		fputs(s.body.c_str(), f); fclose(f);
	}
	std::cout << "Settings schema, validation, atomic edits, exclusions, retries, actions, NTP auth and mode isolation passed\n";
}

/**
 * @file ota_test.cpp
 * @brief Streaming validation and failure-path checks without network or flash access.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/update/ota_image.h"
#include <WebServer.h>
#include <esp_ota_ops.h>
#include <Preferences.h>
#include <esp_bt.h>
#include "../OTA-bot-Mochi/src/update/ap_identity.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

/** @brief Fake application header with the current board's marker. */
static std::vector<uint8_t> binary(bool compatible = true) {
	std::vector<uint8_t> bytes(1024, 0);
	bytes[0] = 0xe9; bytes[12] = 9;
	bytes[32] = 0x32; bytes[33] = 0x54; bytes[34] = 0xcd; bytes[35] = 0xab;
	const char *marker = compatible ? ota_image_marker() : "MOCHI-OTA-v1:wrong-board:16m-v1:END";
	memcpy(bytes.data() + 400, marker, strlen(marker));
	return bytes;
}

/** @brief Begin an isolated session with fresh credentials and no flash failures. */
static WebServer &session() {
	ota_stop();
	test_begin_error = test_write_error = test_end_error = test_select_error = 0;
	test_now = 100;
	ota_begin(test_now);
	assert(ota_status().active && !ota_status().error);
	assert(strlen(ota_status().password) == 12);
	for (const char *p = ota_status().password; *p; ++p) {
		assert(strchr("ABCDEFGHJKLMNPQRSTUVWXYZ23456789", *p));
	}
	auto &server = *WebServer::instance;
	server.host = "192.168.4.1";
	server.headers.clear();
	server.response_headers.clear();
	server.get();
	assert(std::string(server.response_headers["Content-Security-Policy"].c_str()).find("default-src 'none'") != std::string::npos);
	assert(server.response_headers["Cache-Control"] == "no-store");
	std::string page = server.body.c_str();
	const std::string prefix = "setRequestHeader('X-Mochi-Token','";
	size_t start = page.find(prefix);
	assert(start != std::string::npos);
	server.headers["X-Mochi-Token"] = page.substr(start + prefix.size(), 32);
	server.headers["X-Mochi-Size"] = "1024";
	return server;
}

/** @brief Emit multipart events with small chunks to cover header/marker boundaries. */
static void send(WebServer &server, const std::vector<uint8_t> &bytes, size_t chunk = 17) {
#if MOCHI_BOARD_LCD_169
	server.file.filename = "mochi-lcd-169-app.bin";
#else
	server.file.filename = "mochi-amoled-18-v2-app.bin";
#endif
	server.file.name = "firmware";
	server.file.status = UPLOAD_FILE_START; server.upload_callback();
	for (size_t offset = 0; offset < bytes.size(); offset += chunk) {
		size_t size = bytes.size() - offset;
		if (size > chunk) { size = chunk; }
		memcpy(server.file.buf, bytes.data() + offset, size);
		server.file.currentSize = size;
		server.file.status = UPLOAD_FILE_WRITE; server.upload_callback();
	}
	server.file.status = UPLOAD_FILE_END; server.upload_callback();
	server.post();
}

/** @brief Reject malformed sizes without arithmetic overflow. */
static void size_checks() {
	assert(ota_image_size("1024", 1024) == 1024);
	for (const char *text : {"", "0", "287", "1025", "-1", "+1024", " 1024", "1024x", "999999999999999999999999"}) {
		assert(ota_image_size(text, 1024) == 0);
	}
	assert(ota_image_size(nullptr, 1024) == 0);
	assert(ota_image_size("9", 2) == 0);
	std::string largest = std::to_string(std::numeric_limits<size_t>::max());
	assert(ota_image_size(largest.c_str(), std::numeric_limits<size_t>::max()) == std::numeric_limits<size_t>::max());
}

/** @brief No malformed/incompatible/failed upload may select a new boot slot. */
static void failure_checks() {
	auto bytes = binary();
	int selected = test_selects;
	auto &s = session();
	s.headers["X-Mochi-Token"] = "wrong";
	send(s, bytes); assert(s.code == 403 && test_selects == selected);
	session(); s.host = "attacker.example";
	send(s, bytes); assert(s.code == 403 && test_selects == selected);
	session(); send(s, binary(false)); assert(s.code == 400 && test_selects == selected);
	session(); bytes[32] = 0;
	int begins = test_begins;
	send(s, bytes); assert(s.code == 400 && test_begins == begins);
	bytes = binary();
	session(); s.headers["X-Mochi-Size"] = "1023";
	send(s, bytes); assert(s.code == 400 && test_selects == selected);
	session(); s.headers["X-Mochi-Size"] = "1025";
	send(s, bytes); assert(s.code == 400 && test_selects == selected);
	for (int *failure : {&test_begin_error, &test_write_error, &test_end_error}) {
		session(); *failure = 1;
		send(s, bytes); assert(s.code == 400 && test_selects == selected);
	}
	session(); s.headers["X-Mochi-Size"] = "99999999";
	send(s, bytes); assert(s.code == 400 && test_selects == selected);
	session();
	s.file.status = UPLOAD_FILE_ABORTED; s.upload_callback(); s.post();
	assert(s.code == 400 && test_selects == selected);
	ota_stop();
	test_no_slot = true; ota_begin(test_now);
	assert(ota_status().error && WiFi.mode_value == WIFI_OFF);
	ota_stop(); test_no_slot = false;
	test_target.size = 0x140000; ota_begin(test_now);
	assert(ota_status().error); ota_stop(); test_target.size = 0x640000;
	WiFi.ready = false; ota_begin(test_now);
	assert(ota_status().error); ota_stop(); WiFi.ready = true;
}

/** @brief Denied callers cannot consume an upload attempt, abort a handle, or change a visible error. */
static void unauthorized_checks() {
	auto &server = session();
	String token = server.headers["X-Mochi-Token"];
	int begins = test_begins, aborts = test_aborts;
	server.headers["X-Mochi-Token"] = "wrong";
	send(server, binary());
	assert(server.code == 403 && !ota_status().error);
	assert(!ota_status().received && test_begins == begins && test_aborts == aborts);
	server.headers["X-Mochi-Token"] = std::string(token.c_str()) + std::string("\0suffix", 7);
	send(server, binary());
	assert(server.code == 403 && !ota_status().error && test_begins == begins);
	server.headers["X-Mochi-Token"] = token;
	server.file.status = UPLOAD_FILE_START;
	server.upload_callback();
	assert(ota_status().receiving && !ota_status().error);
	auto bytes = binary();
	memcpy(server.file.buf, bytes.data(), 400);
	server.file.currentSize = 400;
	server.file.status = UPLOAD_FILE_WRITE;
	server.upload_callback();
	assert(test_begins == begins + 1 && ota_status().received == 400);
	server.headers["X-Mochi-Token"] = "wrong";
	server.upload_callback();
	assert(server.code == 403 && ota_status().receiving && ota_status().received == 400);
	assert(!ota_status().error && test_aborts == aborts);
	server.headers["X-Mochi-Token"] = token;
	server.file.status = UPLOAD_FILE_ABORTED;
	server.upload_callback();
	server.post();
	const char *error = ota_status().error;
	assert(error && server.code == 400);
	server.headers["X-Mochi-Token"] = "wrong";
	send(server, binary());
	assert(server.code == 403 && ota_status().error == error && test_aborts == aborts + 1);
	ota_stop();
}

/** @brief Upgrade weak legacy AP passwords once, preserving the SSID and refusing corrupt or unwritable records. */
static void identity_checks() {
	char ssid[33] = {}, password[13] = {};
	std::string legacy("\x01" "0123456789abABCDEFGH", 21);
	legacy.push_back(0);
	test_nvs["mochi-ap/identity"] = std::vector<uint8_t>(legacy.begin(), legacy.end());
	test_save_fails = true;
	assert(!ap_identity::load(ssid, password) && test_nvs["mochi-ap/identity"].size() == 22);
	test_save_fails = false;
	assert(ap_identity::load(ssid, password));
	assert(std::string(ssid) == "Mochi-0123456789ab" && strlen(password) == 12);
	unsigned writes = test_flash_writes;
	assert(ap_identity::load(ssid, password) && writes == test_flash_writes);
	test_nvs["mochi-ap/identity"][0] = 99;
	assert(!ap_identity::load(ssid, password) && writes == test_flash_writes);
	test_nvs.erase("mochi-ap/identity");
	test_bt_status = ESP_BT_CONTROLLER_STATUS_IDLE;
	ota_begin(100);
	assert(ota_status().error && !ota_status().url[0]);
	ota_stop();
	test_bt_status = ESP_BT_CONTROLLER_STATUS_ENABLED;
}

/** @brief Timeout, cleanup, streaming header splits, and successful delayed reboot. */
int main() {
	size_checks();
	identity_checks();
	test_save_fails = true; ota_begin(100);
	assert(ota_status().error && WiFi.mode_value == WIFI_OFF); ota_stop(); test_save_fails = false;
	failure_checks();
	unauthorized_checks();
	auto &s = session();
	std::string password = ota_status().password;
	std::string ssid = ota_status().ssid, url = ota_status().url;
	unsigned writes = test_flash_writes;
	ota_stop(); session();
	assert(password == ota_status().password && ssid == ota_status().ssid && !WiFi.persistent_value);
	assert(url != ota_status().url && writes == test_flash_writes);
	test_now += 300000; ota_poll(test_now);
	assert(!ota_status().active && WiFi.mode_value == WIFI_OFF);
	assert(!ota_status().password[0] && !ota_status().ssid[0]);
	session();
	test_now += 300000;
	int begins = test_begins;
	send(s, binary()); assert(s.code == 403 && test_begins == begins);
	session();
	auto bytes = binary();
	send(s, bytes, 1);
	assert(s.code == 200 && ota_status().success && test_written == bytes);
	assert(!ESP.restarted);
	ota_stop(); assert(ota_status().active);
	ota_poll(test_now + 999); assert(!ESP.restarted);
	ota_poll(test_now + 1000); assert(ESP.restarted);
	std::cout << "OTA compatibility, upload, failures, timeout and reboot checks passed\n";
}

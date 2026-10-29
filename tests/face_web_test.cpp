/**
 * @file face_web_test.cpp
 * @brief Local API auth, optimistic edits, storage failures, session modes and timeout.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <WebServer.h>
#include <WiFi.h>
#include <esp_ota_ops.h>
#include <Preferences.h>
#include <cassert>
#include <iostream>
#include <string>

/** @brief Start a face-only session and extract its token without logging credentials. */
static WebServer &session() {
	ota_stop(); test_now = 100; ota_begin(test_now, nullptr, true);
	assert(ota_status().faces && ota_status().active && !ota_status().error);
	auto &s = *WebServer::instance; s.host = "192.168.4.1"; s.headers.clear(); s.get();
	std::string html = s.body.c_str(), prefix = "const token='";
	size_t at = html.find(prefix); assert(at != std::string::npos);
	s.headers["X-Mochi-Token"] = html.substr(at + prefix.size(), 32);
	s.headers["Content-Type"] = "text/plain";
	s.headers["X-Mochi-Faces-Revision"] = String(settings_faces_revision());
	return s;
}
/** @brief Invoke an exact registered route, not an imitation of endpoint logic. */
static void call(WebServer &s, const char *route) {
	assert(s.routes.count(route)); s.routes.at(route)();
}
int main() {
	assert(settings_init() && settings_save());
	auto profile = test_profile;
	test_no_slot = true;
	auto &s = session(); test_no_slot = false; call(s, "GET /faces");
	assert(s.code == 200 && std::string(s.body.c_str()).find("tic_tac_toe") != std::string::npos);
	char mask[FaceFilter::BYTES * 2 + 1]; FaceFilter next;
	assert(next.set(HAPPY, false)); next.hex(mask); s.request_body = mask;
	call(s, "POST /faces"); assert(s.code == 200 && !face_filter().allows(HAPPY) && !settings_faces_dirty());
	assert(test_profile == profile);
	call(s, "POST /faces"); assert(s.code == 409);
	s.headers["X-Mochi-Faces-Revision"] = String(settings_faces_revision());
	unsigned writes = test_flash_writes;
	for (std::string bad : {std::string(FaceFilter::BYTES * 2, '0'), std::string(FaceFilter::BYTES * 2, 'f'), std::string("short"), std::string(2000, 'a')}) {
		s.request_body = bad; call(s, "POST /faces"); assert(s.code == 400 && test_flash_writes == writes);
	}
	s.request_body = mask; s.headers["Content-Type"] = "text/html";
	call(s, "POST /faces"); assert(s.code == 400);
	s.headers["Content-Type"] = "text/plain";
	assert(next.set(LOVE, false)); next.hex(mask); s.request_body = mask;
	test_save_fails = true; call(s, "POST /faces");
	assert(s.code == 507 && settings_faces_dirty() && !face_filter().allows(LOVE));
	test_save_fails = false; s.headers["X-Mochi-Faces-Revision"] = String(settings_faces_revision());
	call(s, "POST /faces"); assert(s.code == 200 && !settings_faces_dirty());
	String token = s.headers["X-Mochi-Token"];
	s.headers["X-Mochi-Token"] = "wrong"; call(s, "GET /faces"); assert(s.code == 403);
	call(s, "POST /faces"); assert(s.code == 403);
	s.headers["X-Mochi-Token"] = token; s.host = "other.example";
	call(s, "POST /faces"); assert(s.code == 403); s.get(); assert(s.code == 403);
	s.host = "192.168.4.1";
	int begins = test_begins, selected = test_selects;
	s.post(); assert(s.code == 403 && test_begins == begins && test_selects == selected);
	s.file.status = UPLOAD_FILE_START; s.upload_callback(); assert(test_begins == begins);
	test_now += 300000; call(s, "POST /faces"); assert(s.code == 403);
	ota_poll(test_now); assert(!ota_status().active && WiFi.mode_value == WIFI_OFF);
	session(); ota_stop(); test_now = 100; ota_begin(100); s.get();
	std::string html = s.body.c_str(), marker = "setRequestHeader('X-Mochi-Token','";
	auto at = html.find(marker); assert(at != std::string::npos);
	s.headers["X-Mochi-Token"] = html.substr(at + marker.size(), 32);
	call(s, "GET /faces"); assert(s.code == 403);
	assert(settings_init() && !face_filter().allows(LOVE) && !face_filter().allows(HAPPY));
	ota_stop();
	std::cout << "Face API auth, invalid masks, revisions, NVS failures, upload isolation and expiry passed\n";
}

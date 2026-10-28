/**
 * @file ota_update.cpp
 * @brief Temporary AP and authenticated, size-bounded inactive-slot uploads.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ota_update.h"
#include "ota_image.h"
#include "ota_page.h"
#include <WiFi.h>
#include <WebServer.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <stdio.h>

namespace {
OtaStatus state;
WebServer server(IPAddress(192, 168, 4, 1), 80);
constexpr uint32_t SESSION_MS = 300000;
uint32_t started = 0, reboot_at = 0;
char token[33] = {};
bool routes_ready = false, wifi_started = false, handle_open = false, file_done = false;
bool attempted = false;
esp_ota_handle_t handle = 0;
const esp_partition_t *target = nullptr;
OtaImageCheck image;
void (*refresh)() = nullptr;

/** @brief Stop writing without changing the boot partition. @param message Recoverable failure. */
void fail(const char *message) {
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	state.receiving = false;
	state.error = message;
}

/** @brief Require a per-session header unavailable to cross-origin HTML forms. @return Authorized. */
bool authorized() {
	return state.active && wifi_started && !state.success &&
		server.hostHeader() == "192.168.4.1" &&
		server.header("X-Mochi-Token") == token &&
		uint32_t(millis() - started) < SESSION_MS;
}

/** @brief Board-specific release asset name. @return Filename. */
const char *filename() {
#if MOCHI_BOARD_LCD_169
	return "mochi-lcd-169-app.bin";
#else
	return "mochi-amoled-18-v2-app.bin";
#endif
}

/** @brief Serve the local form without caching its session token. */
void page() {
	if (!(server.hostHeader() == "192.168.4.1")) { server.send(403, "text/plain", "Use http://192.168.4.1"); return; }
	String html(OTA_PAGE);
	html.replace("%FILE%", filename());
	html.replace("%TOKEN%", token);
	server.sendHeader("Cache-Control", "no-store");
	server.sendHeader("X-Frame-Options", "DENY");
	server.sendHeader("X-Content-Type-Options", "nosniff");
	server.send(200, "text/html", html);
}

/** @brief Initialize exactly one file per request, before any flash write. */
void upload_start() {
	if (attempted) { fail("One upload per session. Close and retry."); return; }
	attempted = true;
	state.error = nullptr;
	state.received = 0;
	state.total = target ? ota_image_size(server.header("X-Mochi-Size").c_str(), target->size) : 0;
	image = OtaImageCheck{};
	if (!state.total || !(server.upload().filename == filename()) || !(server.upload().name == "firmware")) {
		fail("Invalid file or size. Choose the matching release app.bin."); return;
	}
	state.receiving = true;
}

/** @brief Check and write one chunk, retaining the small header until complete. */
void upload_write() {
	HTTPUpload &upload = server.upload();
	if (!state.receiving || state.error) { return; }
	if (upload.currentSize > state.total - state.received) { fail("Image exceeds declared size."); return; }
	size_t previous = image.bytes;
	image.feed(upload.buf, upload.currentSize);
	state.received += upload.currentSize;
	if (!handle_open && image.bytes >= sizeof(image.header)) {
		if (!image.app_header()) { fail("Not an ESP32-S3 application image."); return; }
		if (esp_ota_begin(target, state.total, &handle) != ESP_OK) { fail("Cannot open OTA slot."); return; }
		handle_open = true;
		if (esp_ota_write(handle, image.header, sizeof(image.header)) != ESP_OK) { fail("Flash write failed."); return; }
	}
	size_t skip = previous < sizeof(image.header) ? sizeof(image.header) - previous : 0;
	if (handle_open && upload.currentSize > skip &&
		esp_ota_write(handle, upload.buf + skip, upload.currentSize - skip) != ESP_OK) { fail("Flash write failed."); }
}

/** @brief Handle multipart events; finalization waits for the entire HTTP request. */
void upload() {
	if (!authorized()) { fail("Unauthorized or expired session."); server.client().stop(); return; }
	switch (server.upload().status) {
		case UPLOAD_FILE_START: upload_start(); break;
		case UPLOAD_FILE_WRITE: upload_write(); break;
		case UPLOAD_FILE_END:
			if (state.receiving && !state.error) { file_done = true; state.receiving = false; }
			break;
		case UPLOAD_FILE_ABORTED: fail("Upload interrupted. Reopen update mode."); break;
	}
	if (refresh) { refresh(); }
	// Bound even a client that continually sends data without yielding to the UI loop.
	if (uint32_t(millis() - started) >= SESSION_MS) { fail("Session expired."); server.client().stop(); }
}

/** @brief Validate the complete image before selecting the next boot slot. */
void finish() {
	if (!authorized()) { server.send(403, "text/plain", "Unauthorized or expired session."); return; }
	if (!file_done || !handle_open || state.received != state.total || !image.board_found || state.error) {
		fail(state.error ? state.error : "Incomplete or incompatible image. Reopen update mode.");
		server.send(400, "text/plain", state.error); return;
	}
	esp_err_t result = esp_ota_end(handle);
	handle_open = false;
	if (result != ESP_OK) { fail("Image integrity check failed."); }
	else if (esp_ota_set_boot_partition(target) != ESP_OK) { fail("Cannot select new boot slot."); }
	if (state.error) { server.send(400, "text/plain", state.error); return; }
	state.success = true;
	server.sendHeader("Connection", "close");
	server.send(200, "text/plain", "Firmware verified. Mochi is restarting.");
	reboot_at = millis();
}

/** @brief Generate session-only secrets with the hardware RNG. @param out Hex buffer. @param bytes Random byte count. */
void random_hex(char *out, size_t bytes) {
	uint8_t random[16];
	esp_fill_random(random, bytes);
	for (size_t i = 0; i < bytes; ++i) { snprintf(out + i * 2, 3, "%02x", unsigned(random[i])); }
}

/** @brief Generate eight uniform readable characters; no 0, 1, I, O or lowercase letters. */
void random_password() {
	static constexpr char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
	static_assert(sizeof(alphabet) == 33, "Five random bits require 32 symbols");
	uint8_t random[8];
	esp_fill_random(random, sizeof(random));
	for (size_t i = 0; i < sizeof(random); ++i) { state.password[i] = alphabet[random[i] & 31]; }
	state.password[8] = '\0';
}
}

void ota_begin(uint32_t now, void (*progress)()) {
	if (state.active) { return; }
	state = OtaStatus{};
	state.active = true; started = now; file_done = false;
	attempted = false;
	refresh = progress;
	target = esp_ota_get_next_update_partition(nullptr);
	const esp_partition_t *running = esp_ota_get_running_partition();
	// Only the committed layout is compatible with the marker in release binaries.
	if (!target || !running || target->address == running->address ||
		target->size != 0x640000 || running->size != 0x640000 ||
		!((target->address == 0x10000 && running->address == 0x650000) ||
		  (target->address == 0x650000 && running->address == 0x10000))) {
		fail("Install the OTA layout using USB first."); return;
	}
	random_password();
	random_hex(token, 16);
	snprintf(state.ssid, sizeof(state.ssid), "Mochi-Update-%.6s", token);
	WiFi.persistent(false);
	wifi_started = true;
	if (!WiFi.mode(WIFI_AP) || !WiFi.softAPConfig(IPAddress(192,168,4,1),
		IPAddress(192,168,4,1), IPAddress(255,255,255,0)) ||
		!WiFi.softAP(state.ssid, state.password, 1, false, 1)) {
		fail("Wi-Fi failed. Close and retry."); WiFi.mode(WIFI_OFF); wifi_started = false; return;
	}
	if (!routes_ready) {
		const char *headers[] = {"X-Mochi-Token", "X-Mochi-Size"};
		server.collectHeaders(headers, 2);
		server.on("/", HTTP_GET, page);
		server.on("/update", HTTP_POST, finish, upload);
		server.onNotFound([] { server.send(404, "text/plain", "Not found"); });
		routes_ready = true;
	}
	server.begin();
}

void ota_poll(uint32_t now) {
	if (!state.active) { return; }
	if (state.success) { if (uint32_t(now - reboot_at) >= 1000) { ESP.restart(); } return; }
	if (uint32_t(now - started) >= SESSION_MS) { ota_stop(); return; }
	if (wifi_started) { server.handleClient(); }
	// A malformed/disconnected request must never retain an open flash handle.
	if (handle_open && !state.receiving && !file_done) { fail("Upload interrupted."); }
}

void ota_stop() {
	if (state.success) { return; }
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	if (wifi_started) {
		server.stop();
		WiFi.softAPdisconnect(true);
		WiFi.mode(WIFI_OFF);
	}
	wifi_started = false; file_done = false;
	memset(token, 0, sizeof(token));
	state = OtaStatus{};
}

const OtaStatus &ota_status() { return state; }

/**
 * @file ota_update.cpp
 * @brief Temporary AP and authenticated, size-bounded inactive-slot uploads.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ota_update.h"
#include "ota_image.h"
#include "ota_page.h"
#include "face_web.h"
#include "settings_web.h"
#include "settings_page.h"
#include "ntp_sync.h"
#include "network_web.h"
#include "ap_identity.h"
#include "wifi_scan.h"
#include <WiFi.h>
#include <WebServer.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <stdio.h>

namespace {
OtaStatus state;
WebServer server(80);
constexpr uint32_t SESSION_MS = 300000;
uint32_t started = 0, reboot_at = 0;
char token[33] = {};
bool routes_ready = false, wifi_started = false, handle_open = false, file_done = false;
bool attempted = false;
esp_ota_handle_t handle = 0;
const esp_partition_t *target = nullptr;
OtaImageCheck image;
void (*refresh)() = nullptr;

/** @brief Check both Host and receiving interface; a LAN client cannot impersonate an AP request. */
bool local_host() {
	return server.hostHeader() == state.address && server.client().localIP().toString() == state.address;
}

/** @brief Stop writing without changing the boot partition. @param message Recoverable failure. */
void fail(const char *message) {
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	state.receiving = false;
	state.error = message;
}

/** @brief Require a per-session header unavailable to cross-origin HTML forms. @return Authorized. */
bool authorized() {
	return state.active && wifi_started && !state.success &&
		local_host() &&
		server.header("X-Mochi-Token") == token &&
		uint32_t(millis() - started) < SESSION_MS;
}
/** @brief Apply the existing session boundary to all settings and clock routes. */
bool settings_authorized() {
	face_web::headers(server);
	if (authorized() && state.faces) { return true; }
	server.send(403, "application/json", "{\"error\":\"Session expired or unauthorized\"}"); return false;
}
/** @brief Publish clock progress without revealing network credentials or SSID. */
void ntp_state(int code = 200) {
	server.send(code, "application/json", String("{\"status\":") + face_web::quoted(ntp_sync_status()) +
		",\"busy\":" + (ntp_sync_busy() ? "true" : "false") + "}");
}
/** @brief Validate all four form fields before starting any network activity. */
void ntp_start() {
	if (network_web::form(server, 1) && server.arg("action") == "sync") {
		ntp_state(network_sync_now(millis())); return;
	}
	if (network_config().enabled) { ntp_state(409); return; }
	if (!(server.header("Content-Type") == "application/x-www-form-urlencoded") || server.args() != 4) {
		server.send(400, "application/json", "{\"error\":\"Invalid time form\"}"); return;
	}
	String ssid = server.arg("ssid"), password = server.arg("password"), host = server.arg("server"), zone = server.arg("zone");
	if ((!server.hasArg("password")) || !(zone == "0" || zone == "1") || ssid.length() != strlen(ssid.c_str()) ||
		password.length() != strlen(password.c_str()) || host.length() != strlen(host.c_str())) {
		server.send(400, "application/json", "{\"error\":\"Invalid time fields\"}"); return;
	}
	int code = ntp_sync_start(ssid.c_str(), password.c_str(), host.c_str(), zone == "1" ? 1 : 0, millis());
	ntp_state(code);
}
/** @brief Firmware mode is an explicit authorized action, never an implicit upload bypass. */
void settings_action() {
	if (!(server.arg("action") == "firmware")) { settings_web::action(server); return; }
	if (!(server.header("Content-Type") == "application/x-www-form-urlencoded") || server.args() != 1) {
		server.send(400, "application/json", "{\"error\":\"Invalid action form\"}"); return;
	}
	const esp_partition_t *running = esp_ota_get_running_partition();
	if (!target || !running || target->size != 0x640000 || running->size != 0x640000 ||
		!((target->address == 0x10000 && running->address == 0x650000) ||
		  (target->address == 0x650000 && running->address == 0x10000))) {
		server.send(503, "application/json", "{\"error\":\"Install OTA layout using USB first\"}"); return;
	}
	wifi_scan::stop(); ntp_sync_stop(); sound_init(millis()); state.faces = false; state.error = nullptr;
	server.send(200, "application/json", "{\"firmware\":true}");
}
/** @brief Install settings routes alongside the unchanged face and firmware contracts. */
bool settings_authorized_page() {
	if (state.active && state.faces && wifi_started && local_host() &&
		uint32_t(millis() - started) < SESSION_MS) { return true; }
	server.send(403, "text/plain", "Use the active Mochi settings session."); return false;
}
/** @brief Install settings routes alongside the unchanged face and firmware contracts. */
void settings_page() {
	if (!settings_authorized_page()) { return; }
	String html(SETTINGS_PAGE); html.replace("%TOKEN%", state.lan ? "" : token);
	html.replace("%NETWORK_HTML%", NETWORK_FORM); html.replace("%NETWORK_SCRIPT%", NETWORK_SCRIPT);
	html.replace("%LANG%", settings_get().language ? "it" : "en");
	html.replace("</style>", (String(WEB_STYLE) + "</style>").c_str());
	face_web::headers(server);
	server.sendHeader("Content-Security-Policy", "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self'; base-uri 'none'; form-action 'none'; frame-ancestors 'none'");
	server.send(200, "text/html", html);
}
/** @brief Register settings HTML and authorized APIs. */
void settings_routes() {
	server.on("/settings-ui", HTTP_GET, settings_page);
	server.on("/faces-ui", HTTP_GET, [] {
		if (settings_authorized_page()) { face_web::page(server, state.lan ? "" : token); }
	});
	server.on("/network/scan", HTTP_POST, [] { if (settings_authorized()) { wifi_scan::start(server); } });
	server.on("/network/scan", HTTP_GET, [] { if (settings_authorized()) { wifi_scan::results(server); } });
	server.on("/settings", HTTP_GET, [] { if (settings_authorized()) { settings_web::state(server); } });
	server.on("/settings", HTTP_POST, [] { if (settings_authorized()) { settings_web::save(server); } });
	server.on("/settings/action", HTTP_POST, [] { if (settings_authorized()) { settings_action(); } });
	server.on("/ntp", HTTP_GET, [] { if (settings_authorized()) { ntp_state(); } });
	server.on("/ntp", HTTP_POST, [] { if (settings_authorized()) { ntp_start(); } });
	server.on("/network", HTTP_GET, [] { if (settings_authorized()) { network_web::state(server); } });
	server.on("/network", HTTP_POST, [] { if (settings_authorized()) { network_web::save(server); } });
	server.on("/network/forget", HTTP_POST, [] { if (settings_authorized()) { network_web::forget(server); } });
	server.on("/locations", HTTP_GET, [] { if (settings_authorized()) { network_web::locations(server); } });
	server.on("/locations", HTTP_POST, [] { if (settings_authorized()) { network_web::search(server); } });
	server.on("/weather/refresh", HTTP_POST, [] {
		if (!settings_authorized()) { return; }
		if (!network_web::form(server, 1) || !(server.arg("action") == "refresh")) {
			server.send(400, "application/json", "{\"error\":\"Invalid form\"}"); return;
		}
		network_web::state(server, network_weather_now(millis()));
	});
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
	if (!local_host()) { server.send(403, "text/plain", "Use the address shown on Mochi."); return; }
	if (!state.active || !wifi_started || uint32_t(millis() - started) >= SESSION_MS) {
		server.send(403, "text/plain", "Session expired."); return;
	}
	if (state.faces) { settings_page(); return; }
	String html(OTA_PAGE);
	html.replace("%FILE%", filename());
	html.replace("%TOKEN%", state.lan ? "" : token);
	html.replace("</style>", (String(WEB_STYLE) + "</style>").c_str());
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
	if (!authorized() || state.faces) { fail("Unauthorized or expired session."); server.client().stop(); return; }
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
	if (!authorized() || state.faces) { server.send(403, "text/plain", "Unauthorized or expired session."); return; }
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

/** @brief Register shared handlers once, retaining their existing authorization boundaries. */
void install_routes() {
	if (routes_ready) { return; }
	const char *headers[] = {"X-Mochi-Token", "X-Mochi-Size", "X-Mochi-Faces-Revision", "X-Mochi-Settings-Revision", "Content-Type", "X-Mochi-Network-Revision"};
	server.collectHeaders(headers, 6);
	server.on("/", HTTP_GET, page);
	server.on("/update", HTTP_POST, finish, upload);
	server.on("/faces", HTTP_GET, [] {
		face_web::headers(server);
		if (!authorized() || !state.faces) { server.send(403, "application/json", "{\"error\":\"Session expired or unauthorized\"}"); return; }
		face_web::state(server);
	});
	server.on("/faces", HTTP_POST, [] {
		face_web::headers(server);
		if (!authorized() || !state.faces) { server.send(403, "application/json", "{\"error\":\"Session expired or unauthorized\"}"); return; }
		face_web::save(server);
	});
	server.onNotFound([] { server.send(404, "text/plain", "Not found"); });
	settings_routes(); routes_ready = true;
}
}

void ota_begin(uint32_t now, void (*progress)(), bool faces) {
	if (state.active) { return; }
	state = OtaStatus{};
	state.active = true; started = now; file_done = false;
	state.faces = faces;
	attempted = false;
	refresh = progress;
	target = esp_ota_get_next_update_partition(nullptr);
	const esp_partition_t *running = esp_ota_get_running_partition();
	// Only the committed layout is compatible with the marker in release binaries.
	if (!faces && (!target || !running || target->address == running->address ||
		target->size != 0x640000 || running->size != 0x640000 ||
		!((target->address == 0x10000 && running->address == 0x650000) ||
		  (target->address == 0x650000 && running->address == 0x10000)))) {
		fail("Install the OTA layout using USB first."); return;
	}
	random_hex(token, 16);
	state.lan = network_connected();
	if (!state.lan && !ap_identity::load(state.ssid, state.password)) {
		fail("Cannot load saved Wi-Fi identity. Close and retry."); return;
	}
	network_portal(true, !state.lan);
	if (state.lan) {
		strncpy(state.address, network_address().c_str(), sizeof(state.address) - 1);
		strncpy(state.ssid, network_config().ssid, sizeof(state.ssid) - 1);
		state.password[0] = 0;
	}
	snprintf(state.url, sizeof(state.url), "http://%s/#%s", state.address, token);
	WiFi.persistent(false);
	wifi_started = true;
	if (!state.lan && (!WiFi.mode(network_config().enabled ? WIFI_AP_STA : WIFI_AP) || !WiFi.softAPConfig(IPAddress(192,168,4,1),
		IPAddress(192,168,4,1), IPAddress(255,255,255,0)) ||
		!WiFi.softAP(state.ssid, state.password, 1, false, 1))) {
		fail("Wi-Fi failed. Close and retry."); network_restore_radio(); wifi_started = false; return;
	}
	install_routes();
	server.begin();
}

void ota_poll(uint32_t now) {
	if (!state.active) { return; }
	if (state.success) { if (uint32_t(now - reboot_at) >= 1000) { ESP.restart(); } return; }
	if (uint32_t(now - started) >= SESSION_MS) { ota_stop(); return; }
	if (state.lan && network_connected() && !(network_address() == state.address)) {
		strncpy(state.address, network_address().c_str(), sizeof(state.address) - 1);
		strncpy(state.ssid, network_config().ssid, sizeof(state.ssid) - 1);
		random_hex(token, 16);
		snprintf(state.url, sizeof(state.url), "http://%s/#%s", state.address, token);
	}
	if (wifi_started) { server.handleClient(); }
	wifi_scan::poll(now);
	if (state.faces) {
		ntp_sync_poll(now);
		if (imu_orientation().status == ImuCalibration::Sampling) { eyes_imu_poll(now); }
		sound_poll(now, UiPageEvents{}, false, false, false);
	}
	// A malformed/disconnected request must never retain an open flash handle.
	if (handle_open && !state.receiving && !file_done) { fail("Upload interrupted."); }
}

void ota_stop() {
	if (state.success) { return; }
	wifi_scan::stop();
	if (!network_config().enabled) { ntp_sync_stop(); }
	if (state.faces) { sound_init(millis()); }
	if (handle_open) { esp_ota_abort(handle); handle_open = false; }
	if (wifi_started) {
		server.stop();
		if (!state.lan) { WiFi.softAPdisconnect(true); }
		network_restore_radio();
	}
	network_portal(false);
	wifi_started = false; file_done = false;
	memset(token, 0, sizeof(token));
	state = OtaStatus{};
}

const OtaStatus &ota_status() { return state; }

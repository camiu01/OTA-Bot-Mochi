/**
 * @file face_web.h
 * @brief Same-origin, session-authorized allow-list API; independent of firmware upload.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_page.h"
#include "../settings/settings.h"
#include "../ui/i18n.h"
#include <WebServer.h>

namespace face_web {
/** @brief Common non-cacheable responses for settings and session HTML. */
inline void headers(WebServer &server) {
	server.sendHeader("Cache-Control", "no-store");
	server.sendHeader("X-Frame-Options", "DENY");
	server.sendHeader("X-Content-Type-Options", "nosniff");
}
/** @brief Escape static catalog/localization strings as JSON string values. */
inline String quoted(const char *value) {
	String out("\"");
	for (const char *p = value; *p; ++p) {
		if (*p == '"' || *p == '\\') { out += '\\'; }
		if (uint8_t(*p) >= 32) { out += *p; }
	}
	return out + "\"";
}
/** @brief Return state to both GET and POST, including failed persistence retries. */
inline void state(WebServer &server, int code = 200) {
	char hex[FaceFilter::BYTES * 2 + 1]; face_filter().hex(hex);
	String json = String("{\"enabled\":\"") + hex + "\",\"revision\":" +
		String(settings_faces_revision()) + ",\"saved\":" + (settings_faces_dirty() ? "false" : "true");
	json += ",\"names\":[";
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
		if (i) { json += ','; } json += quoted(eyes_expression_name(Expression(i)));
	}
	json += "],\"labels\":[";
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
		if (i) { json += ','; } json += quoted(tr_expression(i));
	}
	json += "]}";
	headers(server); server.send(code, "application/json", json);
}
/** @brief Render the offline editor, only after its caller checked Host and session mode. */
inline void page(WebServer &server, const char *token) {
	String html(FACE_PAGE);
	html.replace("%TOKEN%", token);
	html.replace("%LANG%", settings_get().language == 1 ? "it" : "en");
	headers(server);
	server.sendHeader("Content-Security-Policy", "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self'; base-uri 'none'; form-action 'none'; frame-ancestors 'none'");
	server.send(200, "text/html", html);
}
/** @brief Reject invalid input before changing RAM or writing the separate NVS record. */
inline void save(WebServer &server) {
	if (!(server.header("Content-Type") == "text/plain")) { server.send(400, "application/json", "{\"error\":\"Invalid content type\"}"); return; }
	if (!(server.header("X-Mochi-Faces-Revision") == String(settings_faces_revision()))) {
		server.send(409, "application/json", "{\"error\":\"Reload current face list\"}"); return;
	}
	String body = server.arg("plain");
	FaceFilter next;
	if (body.length() != FaceFilter::BYTES * 2 || !next.parse_hex(body.c_str())) {
		server.send(400, "application/json", "{\"error\":\"Keep at least one valid face\"}"); return;
	}
	settings_faces_update(next);
	state(server, settings_faces_save() ? 200 : 507);
}
}

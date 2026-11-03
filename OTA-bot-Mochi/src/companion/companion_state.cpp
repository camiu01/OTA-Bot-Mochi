/**
 * @file companion_state.cpp
 * @brief Mutex-owned phone content; callbacks never issue commands or mutate hardware/settings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "companion_state.h"
#include "../ui/utf8_text.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>

namespace {
StaticSemaphore_t storage;
SemaphoreHandle_t mutex = nullptr;
CompanionSnapshot state;
CompanionContact contacts[16];
CompanionHour forecast[24];
char qr[9][97] = {};
CompanionAction action = CompanionAction::None;

/** @brief Copy a UTF-8 prefix into an owned fixed-capacity string. @param text Source. @param out Buffer. @param capacity Capacity. */
void text_copy(const String &text, char *out, size_t capacity) {
	size_t length = utf8_prefix(text.c_str(), text.length(), capacity - 1);
	memcpy(out, text.c_str(), length); out[length] = 0;
}
/** @brief Decode minute-accurate windows without accepting malformed times. @param flags Enable/interval. @param packed Times. @param out Destination. @return Valid. */
bool window(uint32_t flags, uint32_t packed, CompanionWindow &out) {
	uint8_t sh = packed >> 24, sm = packed >> 16, eh = packed >> 8, em = packed;
	if (sh > 23 || eh > 23 || sm > 59 || em > 59 || (flags & 0xffff) > 1) { return false; }
	out = CompanionWindow{uint16_t(sh * 60 + sm), uint16_t(eh * 60 + em), uint16_t(flags >> 16), bool(flags & 1)};
	return true;
}
/** @brief Retain the completed contact batch within the fixed memory budget. @param watch Source. @param complete Completed packet. */
void contact_batch(ChronosESP32 &watch, bool complete) {
	state.contacts = 0;
	int total = watch.getContactCount();
	state.total_contacts = total >= 0 && total <= 255 ? uint8_t(total) : 0;
	if (!complete) { return; }
	state.contacts = state.total_contacts > 16 ? 16 : state.total_contacts;
	for (uint8_t i = 0; i < state.contacts; ++i) {
		const auto &contact = watch.getContact(i);
		text_copy(contact.name, contacts[i].name, sizeof(contacts[i].name));
		text_copy(contact.number, contacts[i].number, sizeof(contacts[i].number));
	}
}
/** @brief Keep QR payloads exact; invalid, embedded-NUL or oversized values clear the slot. @param watch Source. @param index Slot. */
void qr_slot(ChronosESP32 &watch, uint32_t index) {
	if (index >= 9) { return; }
	String value = watch.getQrAt(index);
	state.qr_mask &= ~(uint16_t(1) << index); qr[index][0] = 0;
	if (!value.length() || value.length() > 96 || strlen(value.c_str()) != value.length() ||
		utf8_prefix(value.c_str(), value.length(), 96) != value.length()) { return; }
	memcpy(qr[index], value.c_str(), value.length() + 1);
	state.qr_mask |= uint16_t(1) << index;
}
/** @brief Decode scalar settings from their documented packed fields. @param config Packet. @param a Parameter. @param b Parameter. */
void scalar(Config config, uint32_t a, uint32_t b) {
	if (config == CF_FIND) { ++state.finds; }
	if (config == CF_PBAT && a <= 1 && b <= 100) { state.battery = b; state.phone_charging = a == 1; }
	if (config == CF_CAMERA && b <= 1) { state.camera = b == 1; }
	if (config == CF_LANG && b < 255) { state.language = b; }
	if (config == CF_QUIET && a <= 1) { window(a, b, state.quiet); }
	if (config == CF_SLEEP && a <= 1) { window(a, b, state.sleep); }
	if (config == CF_WATER || config == CF_SED) {
		CompanionWindow value;
		if (window(a, b, value) && (!value.enabled || (value.interval && value.interval <= 1440))) {
			state.reminders[config == CF_WATER ? 0 : 1] = value;
		}
	}
	if (config != CF_ALARM || a >= 8) { return; }
	uint8_t hour = b >> 24, minute = b >> 16, repeat = b >> 8, enabled = b;
	if (hour > 23 || minute > 59 || enabled > 1 || (repeat > 127 && repeat != 128)) { return; }
	state.alarms[a] = Alarm{hour, minute, repeat, enabled == 1};
	state.alarm_mask |= 1U << a;
}
}

bool companion_init() {
	if (!mutex) { mutex = xSemaphoreCreateMutexStatic(&storage); }
	return mutex != nullptr;
}
void companion_connection(bool connected) {
	if (!mutex) { return; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	state.connected = connected;
	if (!connected) {
		state.camera = state.phone_charging = false; state.battery = 255;
		state.contacts = state.total_contacts = 0; state.qr_mask = 0; state.forecast_mask = 0;
		for (auto &contact : contacts) { contact = CompanionContact{}; }
		memset(qr, 0, sizeof(qr));
		action = CompanionAction::None;
	}
	++state.revision;
	xSemaphoreGive(mutex);
}
void companion_config(ChronosESP32 &watch, Config config, uint32_t a, uint32_t b) {
	if (!mutex) { return; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (state.connected) {
		scalar(config, a, b);
		if (config == CF_CONTACT) { contact_batch(watch, a == 1); }
		if (config == CF_QR && a == 0) { qr_slot(watch, b); }
		++state.revision;
	}
	xSemaphoreGive(mutex);
}
CompanionSnapshot companion_snapshot() {
	if (!mutex) { return CompanionSnapshot{}; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	CompanionSnapshot copy = state;
	xSemaphoreGive(mutex); return copy;
}
void companion_alarm_used(uint8_t index) {
	if (!mutex || index >= 8) { return; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (state.alarms[index].repeat == 128) { state.alarms[index].enabled = false; ++state.revision; }
	xSemaphoreGive(mutex);
}
void companion_data(const uint8_t *data, size_t length, int day) {
	if (!mutex || !data || length < 8 || data[0] != 0xea || data[4] != 0x7e || data[5] != 2 ||
		day < 0 || day > 365 || !data[6] || data[6] > 24 || data[7] + data[6] > 24 ||
		length != 8u + size_t(data[6]) * 6u) { return; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (state.connected) {
		for (uint8_t i = 0; i < data[6]; ++i) {
			const uint8_t *p = data + 8 + i * 6;
			uint8_t slot = data[7] + i, icon = p[0] >> 4;
			int temp = p[1] * ((p[0] & 1) ? -1 : 1), wind = p[2] * 256 + p[3];
			state.forecast_mask &= ~(uint32_t(1) << slot);
			if (icon > 7 || temp < -90 || temp > 70 || wind > 500 || p[4] > 100 || p[5] > 30) { continue; }
			forecast[slot] = CompanionHour{int16_t(day), int16_t(wind), int8_t(temp), slot, icon, p[4], p[5]};
			state.forecast_mask |= uint32_t(1) << slot;
		}
		++state.revision;
	}
	xSemaphoreGive(mutex);
}
CompanionContact companion_contact(uint8_t index) {
	CompanionContact copy;
	if (!mutex) { return copy; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (index < state.contacts) { copy = contacts[index]; }
	xSemaphoreGive(mutex); return copy;
}
CompanionHour companion_hour(uint8_t index) {
	CompanionHour copy;
	if (!mutex) { return copy; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (index < 24 && (state.forecast_mask & (uint32_t(1) << index))) { copy = forecast[index]; }
	xSemaphoreGive(mutex); return copy;
}
String companion_qr(uint8_t index) {
	String copy;
	if (!mutex) { return copy; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	if (index < 9 && (state.qr_mask & (uint16_t(1) << index))) { copy = qr[index]; }
	xSemaphoreGive(mutex); return copy;
}
bool companion_request(CompanionAction requested) {
	if (!mutex || requested == CompanionAction::None) { return false; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	bool accepted = state.connected && action == CompanionAction::None &&
		(requested != CompanionAction::Camera || state.camera);
	if (accepted) { action = requested; ++state.revision; }
	xSemaphoreGive(mutex); return accepted;
}
CompanionAction companion_take() {
	if (!mutex) { return CompanionAction::None; }
	xSemaphoreTake(mutex, portMAX_DELAY);
	CompanionAction requested = state.connected ? action : CompanionAction::None;
	action = CompanionAction::None;
	xSemaphoreGive(mutex); return requested;
}

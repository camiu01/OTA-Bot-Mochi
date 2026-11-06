/**
 * @file companion_service.cpp
 * @brief Single-owner main-task actions; schedule edges never run from BLE callbacks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "companion_service.h"
#include "companion_policy.h"
#include "companion_language.h"
#include "../ui/ui_state.h"
#include "../ui/i18n.h"
#include "../settings/settings.h"
#include "../power/backlight.h"
#include "../sound/sound.h"

namespace {
bool finding = false, subscribed = false;
uint32_t find_at = 0, seen_finds = 0, last_find_alert = 0, action_at = 0;
bool alerted = false, acted = false;
uint8_t camera_result = 0;
uint32_t fired[10] = {};
bool has_fired[10] = {};

/** @brief Publish a local dismissible alert using existing bounded notification and mute policies. @param title Localized title. @param now Time. */
void alert(Text title, uint32_t now) {
	Notification item;
	item.app = "MOCHI"; item.title = tr(title); item.message = tr(Text::CompanionDismiss); item.time = "";
	ui_notification(item); backlight_activity(now);
	sound_request(SoundCue::Notification, now);
}
/** @brief Apply a learned app language in RAM only. @param id Opaque language ID. */
void language(uint8_t id) {
	uint8_t local = companion_language_get(id);
	if (local > 1 || local == settings_get().language) { return; }
	CompanionSettings candidate = settings_get(); candidate.language = local;
	settings_update(candidate);
}
/** @brief Execute one queued action and enforce a bounded find-phone timeout. @param watch Transport. @param data Snapshot. @param now Time. */
void commands(ChronosESP32 &watch, const CompanionSnapshot &data, uint32_t now) {
	if (finding && (!data.connected || uint32_t(now - find_at) >= 15000)) {
		if (data.connected) { watch.findPhone(false); }
		finding = false;
	}
	CompanionAction action = companion_take();
	if (action == CompanionAction::StopFind) { watch.findPhone(false); finding = false; return; }
	if (action == CompanionAction::None || (acted && uint32_t(now - action_at) < 500)) { return; }
	acted = true; action_at = now;
	if (action == CompanionAction::FindPhone) { watch.findPhone(true); finding = true; find_at = now; }
	if (action == CompanionAction::Camera) { camera_result = watch.capturePhoto() ? 1 : 2; }
}
/** @brief Reject duplicate or backward-time alert delivery for each independent source. @param index Slot. @param minute Absolute minute. @return First occurrence. */
bool once(uint8_t index, uint32_t minute) {
	if (has_fired[index] && minute <= fired[index]) { return false; }
	has_fired[index] = true; fired[index] = minute; return true;
}
/** @brief Evaluate exact-minute alarms/reminders only after trusted clock synchronization. @param watch Clock. @param data Snapshot. @param now Time. */
void schedules(ChronosESP32 &watch, const CompanionSnapshot &data, uint32_t now) {
	uint32_t stamp = uint32_t(watch.getEpoch() / 60);
	uint8_t hour = watch.getHour(true), minute = watch.getMinute(), weekday = watch.getDayofWeek();
	if (hour > 23 || minute > 59 || weekday > 6) { return; }
	uint16_t local = uint16_t(hour * 60 + minute);
	settings_remote_night(companion_window_active(data.quiet, local) || companion_window_active(data.sleep, local));
	for (uint8_t i = 0; i < 8; ++i) {
		if ((data.alarm_mask & (1U << i)) && companion_alarm_due(data.alarms[i], hour, minute, weekday) && once(i, stamp)) {
			companion_alarm_used(i); alert(Text::CompanionAlarm, now);
		}
	}
	for (uint8_t i = 0; i < 2; ++i) {
		if (companion_reminder_due(data.reminders[i], local) && once(8 + i, stamp)) {
			alert(i == 0 ? Text::CompanionWater : Text::CompanionMove, now);
		}
	}
}
}

void companion_poll(ChronosESP32 &watch, uint32_t now, bool clock_valid) {
	static bool polled = false;
	static uint32_t last_poll = 0;
	if (polled && uint32_t(now - last_poll) < 250) { return; }
	polled = true; last_poll = now;
	CompanionSnapshot data = companion_snapshot();
	if (settings_get().phone_ios) {
		if (finding && data.connected) { watch.findPhone(false); }
		companion_take(); settings_remote_night(false); finding = false; return;
	}
	if (data.connected && !subscribed) { watch.setNotifyBattery(true); }
	subscribed = data.connected;
	language(data.language);
	commands(watch, data, now);
	if (data.finds != seen_finds) {
		seen_finds = data.finds;
		if (data.connected && (!alerted || uint32_t(now - last_find_alert) >= 5000)) {
			alert(Text::CompanionFind, now); last_find_alert = now; alerted = true;
		}
	}
	if (clock_valid) { schedules(watch, data, now); }
	else { settings_remote_night(false); }
}
uint8_t companion_camera_result() { return camera_result; }
bool companion_finding() { return finding; }

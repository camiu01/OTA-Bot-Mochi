/**
 * @file companion_test.cpp
 * @brief Chronos boundary validation, owned snapshots, schedules, actions, language binding and iPhone isolation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/companion/companion_state.h"
#include "../OTA-bot-Mochi/src/companion/companion_policy.h"
#include "../OTA-bot-Mochi/src/companion/companion_service.h"
#include "../OTA-bot-Mochi/src/companion/companion_language.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/ui/ui_state.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include <Preferences.h>
#include <cassert>
#include <iostream>

/** @brief Encode exactly the alarm callback shape emitted by Chronos 1.9.1. @param hour Hour. @param minute Minute. @param repeat Weekday mask. @return Packed enabled alarm. */
static uint32_t alarm_value(uint8_t hour, uint8_t minute, uint8_t repeat) {
	return (uint32_t(hour) << 24) | (uint32_t(minute) << 16) | (uint32_t(repeat) << 8) | 1;
}
/** @brief Check Monday-first repeats, midnight wrap, and interval anchoring. */
static void policy_checks() {
	Alarm alarm{7, 30, 1, true};
	assert(companion_alarm_due(alarm, 7, 30, 1));
	assert(!companion_alarm_due(alarm, 7, 30, 0));
	alarm.repeat = 64;
	assert(companion_alarm_due(alarm, 7, 30, 0));
	assert(!companion_alarm_due(alarm, 7, 30, 1));
	alarm.repeat = 128;
	assert(companion_alarm_due(alarm, 7, 30, 6));
	assert(!companion_alarm_due(alarm, 7, 31, 6));
	CompanionWindow night{22 * 60 + 15, 7 * 60 + 45, 30, true};
	assert(!companion_window_active(night, 22 * 60 + 14));
	assert(companion_window_active(night, 22 * 60 + 15));
	assert(companion_window_active(night, 7 * 60 + 44));
	assert(!companion_window_active(night, 7 * 60 + 45));
	assert(companion_reminder_due(night, 15));
	assert(!companion_reminder_due(night, 16));
}
/** @brief Reject invalid scalars, then retain independent contact and QR copies. @param watch Source. */
static void boundary_checks(ChronosESP32 &watch) {
	companion_connection(true);
	companion_config(watch, CF_PBAT, 2, 90);
	companion_config(watch, CF_PBAT, 1, 101);
	assert(companion_snapshot().battery == 255);
	companion_config(watch, CF_PBAT, 1, 82);
	assert(companion_snapshot().battery == 82 && companion_snapshot().phone_charging);
	companion_config(watch, CF_ALARM, 8, alarm_value(7, 30, 127));
	companion_config(watch, CF_ALARM, 0, alarm_value(24, 30, 127));
	assert(!companion_snapshot().alarm_mask);
	watch.contact_count = 200;
	watch.contacts[0] = Contact{"Alice", "+39123456"};
	companion_config(watch, CF_CONTACT, 0, 200);
	assert(companion_snapshot().contacts == 0);
	companion_config(watch, CF_CONTACT, 1, 200);
	assert(companion_snapshot().contacts == 16 && companion_snapshot().total_contacts == 200);
	watch.contacts[0].name = "Changed";
	assert(std::string(companion_contact(0).name) == "Alice");
	assert(!companion_contact(16).name[0]);
	watch.qr_links[2] = "https://example.com/test";
	companion_config(watch, CF_QR, 0, 2);
	assert(companion_qr(2) == "https://example.com/test");
	watch.qr_links[2] = std::string(97, 'x');
	companion_config(watch, CF_QR, 0, 2);
	assert(!companion_qr(2).length());
	watch.qr_links[2] = std::string("safe\0hidden", 11);
	companion_config(watch, CF_QR, 0, 2);
	assert(!companion_qr(2).length());
	companion_config(watch, CF_QR, 0, UINT32_MAX);
}
/** @brief Validate the hourly packet before copying any forecast content. */
static void hourly_checks() {
	uint8_t packet[] = {0xea, 0, 0, 0xfe, 0x7e, 2, 2, 10, 0x10, 21, 0, 12, 80, 2, 0x31, 5, 0, 4, 90, 1};
	companion_data(packet, sizeof(packet), 0);
	assert(companion_snapshot().forecast_mask == ((1U << 10) | (1U << 11)));
	assert(companion_hour(10).temp == 21 && companion_hour(11).temp == -5);
	assert(companion_hour(10).day == 0 && companion_hour(10).wind == 12);
	uint32_t revision = companion_snapshot().revision;
	companion_data(packet, sizeof(packet) - 1, 0);
	assert(companion_snapshot().revision == revision);
	packet[12] = 200;
	companion_data(packet, sizeof(packet), 0);
	assert(!(companion_snapshot().forecast_mask & (1U << 10)));
	assert(companion_snapshot().forecast_mask & (1U << 11));
	packet[6] = 255;
	companion_data(packet, sizeof(packet), 0);
	assert(!companion_hour(24).day);
}
/** @brief Learn only explicit EN/IT associations and preserve old bindings on a failed save. @param watch Source. */
static void language_checks(ChronosESP32 &watch) {
	assert(!companion_language_init());
	assert(companion_language_get(12) == 255);
	test_save_fails = true;
	assert(!companion_language_bind(12, 1) && companion_language_get(12) == 255);
	test_save_fails = false;
	assert(companion_language_bind(12, 1));
	assert(companion_language_init() && companion_language_get(12) == 1);
	assert(!companion_language_bind(255, 0) && !companion_language_bind(0, 2));
	companion_config(watch, CF_LANG, 0, 12);
	unsigned writes = test_flash_writes;
	companion_poll(watch, 1000, false);
	assert(settings_get().language == 1 && test_flash_writes == writes);
	assert(companion_language_bind(12, 0));
	assert(companion_language_get(12) == 0);
}
/** @brief Main-task commands remain bounded and do not run from a callback. @param watch Transport. */
static void command_checks(ChronosESP32 &watch) {
	assert(watch.battery_subscribed);
	assert(companion_request(CompanionAction::FindPhone));
	assert(!companion_request(CompanionAction::FindPhone) && watch.find_commands == 0);
	companion_poll(watch, 2000, false);
	assert(watch.phone_finding && companion_finding());
	companion_poll(watch, 16999, false);
	assert(watch.phone_finding);
	companion_poll(watch, 17250, false);
	assert(!watch.phone_finding && !companion_finding());
	assert(!companion_request(CompanionAction::Camera));
	companion_config(watch, CF_CAMERA, 0, 1); watch.camera_ready = true;
	assert(companion_request(CompanionAction::Camera));
	companion_poll(watch, 18000, false);
	assert(watch.photos == 1 && companion_camera_result() == 1);
	uint32_t alerts = ui_page_events().notifications;
	companion_config(watch, CF_FIND, 0, 0);
	companion_poll(watch, 18500, false);
	assert(ui_page_events().notifications == alerts + 1);
	companion_config(watch, CF_FIND, 0, 0);
	companion_poll(watch, 19000, false);
	assert(ui_page_events().notifications == alerts + 1);
}
/** @brief Alarms fire once per minute, one-shots disable locally, and quiet windows preserve user settings. @param watch Clock. */
static void schedule_checks(ChronosESP32 &watch) {
	watch.local_hour = 7; watch.local_minute = 30; watch.weekday = 1; watch.epoch = 600000;
	uint32_t alerts = ui_page_events().notifications;
	companion_config(watch, CF_ALARM, 0, alarm_value(7, 30, 128));
	companion_poll(watch, 20000, false);
	assert(ui_page_events().notifications == alerts);
	companion_poll(watch, 20500, true);
	assert(ui_page_events().notifications == alerts + 1 && !companion_snapshot().alarms[0].enabled);
	companion_poll(watch, 21000, true);
	assert(ui_page_events().notifications == alerts + 1);
	uint32_t times = (uint32_t(7) << 24) | (uint32_t(15) << 16) | (uint32_t(8) << 8) | 45;
	companion_config(watch, CF_QUIET, 1, times);
	companion_poll(watch, 21500, true);
	assert(settings_night() && !settings_get().night);
	watch.local_hour = 8; watch.local_minute = 45; watch.epoch += 75 * 60;
	companion_poll(watch, 22000, true);
	assert(!settings_night());
	companion_config(watch, CF_WATER, (uint32_t(30) << 16) | 1, (uint32_t(8) << 24) | (uint32_t(10) << 8));
	watch.local_hour = 9; watch.local_minute = 0; watch.epoch += 15 * 60;
	companion_poll(watch, 22500, true);
	assert(ui_page_events().notifications == alerts + 2);
	companion_poll(watch, 23000, true);
	assert(ui_page_events().notifications == alerts + 2);
}
/** @brief Disconnect clears personal caches and capabilities; iPhone mode cannot execute queued Chronos commands. @param watch Transport. */
static void isolation_checks(ChronosESP32 &watch) {
	companion_connection(false);
	assert(!companion_snapshot().camera && companion_snapshot().battery == 255);
	assert(!companion_snapshot().contacts && !companion_snapshot().qr_mask && !companion_snapshot().forecast_mask);
	assert(!companion_request(CompanionAction::FindPhone));
	companion_config(watch, CF_PBAT, 0, 99);
	assert(companion_snapshot().battery == 255);
	companion_connection(true);
	assert(companion_request(CompanionAction::FindPhone));
	CompanionSettings candidate = settings_get(); candidate.phone_ios = true;
	assert(settings_update(candidate));
	unsigned commands = watch.find_commands;
	companion_poll(watch, 24000, true);
	assert(watch.find_commands == commands && !settings_night());
	assert(companion_take() == CompanionAction::None);
}
/** @brief Exercise the real companion state and main-task service with a controlled phone. @return Success. */
int main() {
	assert(ui_state_init() && settings_init() && companion_init());
	Arduino_GFX gfx; eyes_init(&gfx); settings_apply();
	ChronosESP32 watch;
	policy_checks(); boundary_checks(watch); hourly_checks(); language_checks(watch);
	command_checks(watch); schedule_checks(watch); isolation_checks(watch);
	std::cout << "Chronos validation, owned caches, schedules, commands, language and iPhone isolation passed\n";
	return 0;
}

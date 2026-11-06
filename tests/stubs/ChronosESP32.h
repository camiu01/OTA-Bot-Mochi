/**
 * @file ChronosESP32.h
 * @brief Controlled Chronos data source for native UI tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "Arduino.h"
#define CS_ICON_DATA_SIZE 288

enum Config { CF_TIME, CF_HR24, CF_WEATHER, CF_NAV_DATA, CF_NAV_ICON, CF_OTHER, CF_MUSIC,
	CF_FIND, CF_PBAT, CF_CAMERA, CF_LANG, CF_QUIET, CF_SLEEP, CF_WATER, CF_SED, CF_ALARM, CF_CONTACT, CF_QR };
enum Control { MUSIC_PLAY = 0x9D00, MUSIC_PAUSE = 0x9D01, MUSIC_PREVIOUS = 0x9D02, MUSIC_NEXT = 0x9D03, MUSIC_TOGGLE = 0x9900, VOLUME_UP = 0x99A1, VOLUME_DOWN = 0x99A2 };
struct MusicInfo { uint8_t state = 0; String title, artist; };
struct Notification { int icon = 0; String app, time, title, message; };
struct Weather { int icon = 0, day = 0, temp = 0, high = 0, low = 0, pressure = 0, uv = 0; };
struct Alarm { uint8_t hour = 0, minute = 0, repeat = 0; bool enabled = false; };
struct Contact { String name, number; };
struct Navigation {
	bool active = false, isNavigation = false, hasIcon = false;
	String distance, duration, eta, title, directions, speed;
	uint8_t icon[CS_ICON_DATA_SIZE] = {};
	uint32_t iconCRC = 0;
};
class ChronosESP32 {
public:
	Contact contacts[20];
	int contact_count = 0;
	String qr_links[9];
	Contact &getContact(int index) { return contacts[index]; }
	int getContactCount() { return contact_count; }
	String getQrAt(int index) { return qr_links[index]; }
	bool phone_finding = false, camera_ready = false, battery_subscribed = false;
	unsigned find_commands = 0, photos = 0;
	void findPhone(bool active) { phone_finding = active; ++find_commands; }
	bool capturePhoto() { if (!camera_ready) { return false; } ++photos; return true; }
	void setNotifyBattery(bool enabled) { battery_subscribed = enabled; }
	uint32_t epoch = 0;
	uint8_t local_minute = 0, weekday = 1;
	int year_day = 0;
	uint32_t getEpoch() { return epoch; }
	int getMinute() { return local_minute; }
	int getDayofWeek() { return weekday; }
	int getDayofYear() { return year_day; }
	uint8_t reported_battery = 0;
	bool reported_charging = false;
	unsigned battery_updates = 0;
	/** @brief Capture the main-task battery setter without simulating BLE transmission. */
	void setBattery(uint8_t level, bool charging = false) {
		reported_battery = level; reported_charging = charging; ++battery_updates;
	}
	unsigned controls = 0;
	Control last_control = MUSIC_PLAY;
	void musicControl(Control command) { last_control = command; ++controls; }
	MusicInfo music;
	MusicInfo &getMusicInfo() { return music; }
	Weather weather[7] = {};
	int weather_count = 0;
	String city = "Test City";
	Navigation navigation;
	bool hour24 = true;
	uint8_t local_hour = 13;
	int getHour(bool) { return local_hour; }
	String seconds = ":30";
	String analog_time = "13:45:30";
	int getWeatherCount() { return weather_count; }
	Weather &getWeatherAt(int index) { return weather[index]; }
	String getWeatherCity() { return city; }
	String getWeatherTime() { return "13:45"; }
	Navigation &getNavigation() { return navigation; }
	const uint8_t *getNavigationIcon() { return navigation.icon; }
	String getHourZ() { return hour24 ? "13" : "01"; }
	String getAmPmC() { return hour24 ? "" : "PM"; }
	String getTime(const char *format) {
		if (strcmp(format, "%H:%M:%S") == 0) { return analog_time; }
		if (strcmp(format, ":%M") == 0) { return ":45"; }
		if (strcmp(format, ":%S") == 0) { return seconds; }
		return "13 / 10 / 2026";
	}
};

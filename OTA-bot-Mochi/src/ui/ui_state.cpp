/**
 * @file ui_state.cpp
 * @brief Bounded BLE data ownership; sensitive payloads are never logged.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ui_state.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace {
StaticSemaphore_t lock_storage;
SemaphoreHandle_t lock = nullptr;
UiSnapshot state;
UiPageEvents page_events;
uint8_t icon_parts = 0;
uint32_t assembling_crc = 0;
bool forecast_received = false;
uint32_t forecast_at = 0;

/**
 * @brief Limit incoming text to a bounded amount of retained memory.
 * @param text Incoming text.
 * @param limit Maximum retained bytes.
 * @return Bounded text.
 */
String bounded(const String &text, unsigned int limit) {
	return text.substring(0, limit);
}
}

/** @brief Create the statically allocated mutex. @return True if available. */
bool ui_state_init() {
	lock = xSemaphoreCreateMutexStatic(&lock_storage);
	return lock != nullptr;
}

/** @brief Copy the state outside the rendering task. @return Owned state copy. */
UiSnapshot ui_snapshot() {
	if (!lock) { return UiSnapshot{}; }
	xSemaphoreTake(lock, portMAX_DELAY);
	UiSnapshot copy = state;
	xSemaphoreGive(lock);
	return copy;
}

/** @brief Read the state revision. @return Current revision, or zero before init. */
uint32_t ui_revision() {
	if (!lock) { return 0; }
	xSemaphoreTake(lock, portMAX_DELAY);
	uint32_t revision = state.revision;
	xSemaphoreGive(lock);
	return revision;
}

/** @brief Copy event counters under the snapshot lock. @return Coherent page events. */
UiPageEvents ui_page_events() {
	if (!lock) { return UiPageEvents{}; }
	xSemaphoreTake(lock, portMAX_DELAY);
	UiPageEvents events = page_events;
	xSemaphoreGive(lock);
	return events;
}

/** @brief Update Bluetooth state. @param connected Connection flag. */
void ui_connection(bool connected) {
	if (!lock) { return; }
	xSemaphoreTake(lock, portMAX_DELAY);
	state.connected = connected;
	if (!connected) {
		state.calling = false;
		page_events.call_active = false;
		state.caller = "";
		state.navigation.active = false;
		page_events.route_active = false;
		state.has_icon = false;
		icon_parts = 0;
	}
	++state.revision;
	xSemaphoreGive(lock);
}

/** @brief Retain the most recent notification. @param notification Incoming alert. */
void ui_notification(const Notification &notification) {
	if (!lock) { return; }
	xSemaphoreTake(lock, portMAX_DELAY);
	state.notification.app = bounded(notification.app, 64);
	state.notification.title = bounded(notification.title, 160);
	state.notification.message = bounded(notification.message, 1024);
	state.notification.time = bounded(notification.time, 16);
	state.notification.icon = notification.icon;
	state.notification_received = millis();
	state.has_notification = true;
	++page_events.notifications;
	++state.revision;
	xSemaphoreGive(lock);
}

/** @brief Update the call overlay. @param caller Caller label. @param active Ringer flag. */
void ui_ringer(const String &caller, bool active) {
	if (!lock) { return; }
	xSemaphoreTake(lock, portMAX_DELAY);
	if (active && !state.calling) { ++page_events.calls; }
	page_events.call_active = active;
	state.calling = active;
	state.caller = active ? bounded(caller, 128) : String("");
	++state.revision;
	xSemaphoreGive(lock);
}

/**
 * @brief Copy a weather packet, keeping optional forecasts distinct.
 * @param watch Chronos source.
 * @param a Packet kind.
 * @param b City update flag.
 */
static void weather_update(ChronosESP32 &watch, uint32_t a, uint32_t b) {
	if (a == 2) { ++state.forecast_packets; forecast_received = true; forecast_at = millis(); }
	if (a == 1 || a == 2) {
		int count = watch.getWeatherCount();
		if (a == 1 || count > 0) { state.weather_count = count > 3 ? 3 : (count > 0 ? count : 0); }
		for (uint8_t i = 0; i < state.weather_count; ++i) {
			state.weather[i] = watch.getWeatherAt(i);
		}
		if (a == 1) {
			++state.weather_packets;
			state.has_weather = state.weather_count > 0 && state.weather[0].temp >= -90 && state.weather[0].temp <= 70;
			state.weather_received = millis();
			state.weather_time = bounded(watch.getWeatherTime(), 16);
		}
		state.has_forecast = state.has_weather && forecast_received && uint32_t(millis() - forecast_at) < 10800000;
	}
	if (b) { ++state.city_packets; state.city = bounded(watch.getWeatherCity(), 64); }
}

/**
 * @brief Retain complete route data and icon packets.
 * @param watch Chronos source.
 * @param config Event type.
 * @param a Event parameter.
 * @param b Completed icon CRC.
 */
void ui_config(ChronosESP32 &watch, Config config, uint32_t a, uint32_t b) {
	if (!lock) { return; }
	if (config != CF_WEATHER && config != CF_NAV_DATA && config != CF_NAV_ICON &&
		config != CF_TIME && config != CF_HR24) { return; }
	xSemaphoreTake(lock, portMAX_DELAY);
	if (config == CF_WEATHER) {
		weather_update(watch, a, b);
	} else if (config == CF_NAV_DATA) {
		const Navigation &nav = watch.getNavigation();
		bool route_active = nav.active && nav.isNavigation;
		if (route_active && !page_events.route_active) { ++page_events.route_starts; }
		page_events.route_active = route_active;
		state.navigation.active = nav.active;
		state.navigation.isNavigation = nav.isNavigation;
		state.navigation.hasIcon = nav.hasIcon;
		state.navigation.iconCRC = nav.iconCRC;
		state.navigation.directions = bounded(nav.directions, 256);
		state.navigation.title = bounded(nav.title, 64);
		state.navigation.distance = bounded(nav.distance, 32);
		state.navigation.eta = bounded(nav.eta, 48);
		state.navigation.duration = bounded(nav.duration, 32);
		state.navigation.speed = bounded(nav.speed, 24);
		if (!nav.active) { state.has_icon = false; }
	} else if (config == CF_NAV_ICON && a <= 2) {
		if (a == 0 || b != assembling_crc) { icon_parts = 0; assembling_crc = b; }
		icon_parts |= uint8_t(1 << a);
		if (icon_parts == 7) {
			memcpy(state.icon, watch.getNavigationIcon(), sizeof(state.icon));
			state.icon_crc = b;
			state.has_icon = true;
		}
	}
	++state.revision;
	xSemaphoreGive(lock);
}

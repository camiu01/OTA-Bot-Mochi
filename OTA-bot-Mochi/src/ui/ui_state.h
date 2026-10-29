/**
 * @file ui_state.h
 * @brief Owned snapshots crossing the BLE callback and rendering tasks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <ChronosESP32.h>

struct UiPageEvents {
	uint32_t notifications = 0;
	uint32_t route_starts = 0;
	bool route_active = false;
	uint32_t calls = 0;
	bool call_active = false;
};

struct UiSnapshot {
	bool connected = false;
	bool calling = false;
	bool has_notification = false;
	uint8_t notification_count = 0, notification_index = 0;
	bool has_weather = false;
	bool has_forecast = false;
	bool weather_internet = false;
	bool has_icon = false;
	String caller;
	Notification notification;
	Weather weather[3] = {};
	uint8_t weather_count = 0;
	String city;
	String weather_time;
	uint32_t weather_received = 0;
	uint32_t notification_received = 0;
	Navigation navigation;
	uint8_t icon[CS_ICON_DATA_SIZE] = {};
	uint32_t icon_crc = 0;
	uint32_t revision = 0;
	uint32_t weather_packets = 0, forecast_packets = 0, city_packets = 0;
};

/** @brief Initialize the snapshot lock before starting BLE. @return Initialization success. */
bool ui_state_init();
/** @brief Copy the owned snapshot. @return A thread-safe UI snapshot. */
UiSnapshot ui_snapshot();
/** @brief Read the current revision without copying text. @return Snapshot revision. */
uint32_t ui_revision();
/** @brief Read page-selection events without copying payloads. @return Event counters and route state. */
UiPageEvents ui_page_events();
/** @brief Cache connection changes. @param connected Current connection state. */
void ui_connection(bool connected);
/** @brief Store the latest alert. @param notification Received notification. */
void ui_notification(const Notification &notification);
/** @brief Browse retained alerts, newest first. @param step Positive selects older, negative newer. @param expected_revision Reject stale UI actions, or UINT32_MAX for an unconditional action. */
void ui_notification_step(int8_t step, uint32_t expected_revision = UINT32_MAX);
/** @brief Remove the displayed alert locally, then select the next retained alert. @param expected_revision Reject stale UI actions, or UINT32_MAX for an unconditional action. */
void ui_notification_dismiss(uint32_t expected_revision = UINT32_MAX);
/** @brief Store or clear an incoming call. @param caller Caller label. @param active Ringer state. */
void ui_ringer(const String &caller, bool active);
/**
 * @brief Cache finalized Chronos configuration data without drawing from BLE.
 * @param watch Chronos source.
 * @param config Configuration event.
 * @param a Event parameter.
 * @param b Event parameter.
 */
void ui_config(ChronosESP32 &watch, Config config, uint32_t a, uint32_t b);
/** @brief Publish validated Internet weather from the main task; preserve the separate phone cache. */
void ui_network_weather(const Weather (&days)[3], const char *city, const char *time, uint32_t received);
/** @brief Return to the retained phone weather after disabling/changing Internet weather. */
void ui_network_weather_clear();

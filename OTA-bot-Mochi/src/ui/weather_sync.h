/**
 * @file weather_sync.h
 * @brief Rate-limited app synchronization, not a direct weather-provider request.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
class WeatherSyncPolicy {
	bool connected_before = false, pending = false, tried = false;
	bool last_connected = false, last_subscribed = false;
	uint32_t connected_at = 0, last_try = 0;
public:
	/** @brief Queue one manual refresh. */
	void request() { pending = true; }
	/** @brief Wait for info/subscription and limit sync requests to 30 seconds. @param now Time. @param connected BLE. @param subscribed Notifications ready. @return Send request now. */
	bool update(uint32_t now, bool connected, bool subscribed) {
		last_connected = connected; last_subscribed = subscribed;
		if (!connected) { connected_before = false; pending = false; return false; }
		if (!connected_before) { connected_before = true; connected_at = now; pending = true; }
		if (!pending || !subscribed || uint32_t(now - connected_at) < 4000 ||
			(tried && uint32_t(now - last_try) < 30000)) { return false; }
		pending = false; tried = true; last_try = now;
		return true;
	}
	/** @brief Last observed link state for USB diagnostics. @param connected Output BLE. @param subscribed Output subscription. */
	void link(bool &connected, bool &subscribed) const { connected = last_connected; subscribed = last_subscribed; }
};
/** @brief Main-task refresh requested by touch/USB. */
void weather_request();
/** @brief Consume pending app synchronization when allowed. @param now Time. @param connected BLE. @param subscribed Ready. @return Request now. */
bool weather_sync_due(uint32_t now, bool connected, bool subscribed);
/** @brief Last observed link state for USB diagnostics. @param connected Output BLE. @param subscribed Output subscription. */
void weather_sync_link(bool &connected, bool &subscribed);

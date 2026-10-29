/**
 * @file weather_sync.cpp
 * @brief Single main-loop app-sync request mailbox.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "weather_sync.h"
static WeatherSyncPolicy sync;
static void (*network_refresh)() = nullptr;
void weather_bind_network(void (*request)()) { network_refresh = request; }
/** @brief Queue a refresh without invoking BLE from touch/USB modules. */
void weather_request() { sync.request(); if (network_refresh) { network_refresh(); } }
/** @brief Apply debounce/subscription policy. @param now Time. @param connected BLE. @param subscribed Ready. @return Send. */
bool weather_sync_due(uint32_t now, bool connected, bool subscribed) { return sync.update(now, connected, subscribed); }
/** @brief Expose the last observed BLE/subscription state for diagnostics. @param connected Output BLE. @param subscribed Output subscription. */
void weather_sync_link(bool &connected, bool &subscribed) { sync.link(connected, subscribed); }

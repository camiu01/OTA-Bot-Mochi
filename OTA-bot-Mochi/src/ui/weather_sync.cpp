/**
 * @file weather_sync.cpp
 * @brief Single main-loop app-sync request mailbox.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "weather_sync.h"
static WeatherSyncPolicy sync;
/** @brief Queue a refresh without invoking BLE from touch/USB modules. */
void weather_request() { sync.request(); }
/** @brief Apply debounce/subscription policy. @param now Time. @param connected BLE. @param subscribed Ready. @return Send. */
bool weather_sync_due(uint32_t now, bool connected, bool subscribed) { return sync.update(now, connected, subscribed); }

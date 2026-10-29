/**
 * @file ntp_sync_test.cpp
 * @brief Manual-only SNTP, bounded station lifetime, UTC/Rome DST and callback isolation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ntp_sync.h"
#include <WiFi.h>
#include <esp_sntp.h>
#include <cassert>
#include <cstring>
#include <iostream>

static int applied = 0;
static tm last{};
static void clock_set(const tm &value) { last = value; ++applied; }
static void receive(long epoch) { timeval tv{}; tv.tv_sec = epoch; sntp_sync_time(&tv); }
/** @brief NTP packets are not applied on the TCP/IP task; main-task polling owns the clock. */
static void check_time(long epoch, int hour, int minute, int second, uint8_t zone = 1) {
	ntp_sync_stop(); test_now += 100;
	assert(ntp_sync_start("fixture-net", "fixture-pass", "pool.ntp.org", zone, test_now) == 202);
	WiFi.connection = WL_CONNECTED; ntp_sync_poll(test_now + 1);
	assert(test_sntp_active && !WiFi.persistent_value && !WiFi.reconnect);
	int before = applied; receive(epoch); assert(applied == before);
	ntp_sync_poll(test_now + 2);
	assert(applied == before + 1 && last.tm_hour == hour && last.tm_min == minute && last.tm_sec == second);
	assert(!ntp_sync_busy() && !test_sntp_active && WiFi.mode_value == WIFI_AP);
	assert(strcmp(ntp_sync_status(), "synced") == 0);
	receive(epoch); ntp_sync_poll(test_now + 3); assert(applied == before + 1);
}
int main() {
	ntp_sync_poll(1000); assert(WiFi.station_begins == 0);
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, 0) == 503);
	ntp_sync_bind(clock_set);
	for (const char *ssid : {"", "123456789012345678901234567890123", "bad\nname"}) {
		assert(ntp_sync_start(ssid, "", "pool.ntp.org", 0, 0) == 400);
	}
	for (const char *host : {"", "https://pool.ntp.org", "host:123", "bad host", ".host", "host."}) {
		assert(ntp_sync_start("fixture-net", "", host, 0, 0) == 400);
	}
	assert(ntp_sync_start("fixture-net", "short", "pool.ntp.org", 0, 0) == 400);
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 2, 0) == 400);
	assert(WiFi.station_begins == 0);
	uint32_t start = UINT32_MAX - 1000;
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, start) == 202);
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, start) == 409);
	ntp_sync_poll(start + 24999); assert(ntp_sync_busy());
	ntp_sync_poll(start + 25000); assert(!ntp_sync_busy() && strcmp(ntp_sync_status(), "timeout") == 0);
	check_time(1768478400, 13, 0, 0); check_time(1784116800, 14, 0, 0);
	check_time(1774745999, 1, 59, 59); check_time(1774746000, 3, 0, 0);
	check_time(1792889999, 2, 59, 59); check_time(1792890000, 2, 0, 0);
	check_time(1784116800, 12, 0, 0, 0);
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, test_now) == 202);
	WiFi.connection = WL_CONNECTED; ntp_sync_poll(test_now);
	int before = applied; receive(1600000000); ntp_sync_poll(test_now);
	assert(applied == before && strcmp(ntp_sync_status(), "invalid_time") == 0);
	assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, test_now) == 202);
	WiFi.connection = WL_CONNECTED; ntp_sync_poll(test_now); ntp_sync_stop();
	assert(!test_sntp_active && !ntp_sync_busy() && WiFi.mode_value == WIFI_AP);
	receive(1784116800); ntp_sync_poll(test_now); assert(applied == before);
	WiFi.ready = false; assert(ntp_sync_start("fixture-net", "", "pool.ntp.org", 0, test_now) == 503);
	WiFi.ready = true; ntp_sync_stop();
	WiFi.connection = WL_CONNECTED; WiFi.mode(WIFI_STA); int disconnected = WiFi.station_disconnects;
	assert(ntp_sync_connected("pool.ntp.org", 1, test_now) == 202);
	ntp_sync_poll(test_now); receive(1784116800); ntp_sync_poll(test_now + 1);
	assert(!ntp_sync_busy() && WiFi.mode_value == WIFI_STA && WiFi.station_disconnects == disconnected);
	assert(ntp_sync_connected("pool.ntp.org", 1, test_now) == 202);
	ntp_sync_poll(test_now); ntp_sync_stop();
	assert(WiFi.mode_value == WIFI_STA && WiFi.station_disconnects == disconnected);
	std::cout << "Manual NTP, validation, timeout/rollover, main-task clock, UTC/Rome DST and station cleanup passed\n";
}

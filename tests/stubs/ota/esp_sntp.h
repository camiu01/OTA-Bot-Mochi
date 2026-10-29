/** @brief Controlled SNTP transport; no network or system clock is touched. */
#pragma once
#include <sys/time.h>
constexpr int SNTP_OPMODE_POLL = 0;
inline int test_sntp_starts = 0, test_sntp_stops = 0;
inline bool test_sntp_active = false;
extern "C" void sntp_sync_time(timeval *);
inline void esp_sntp_stop() { ++test_sntp_stops; test_sntp_active = false; }
inline void esp_sntp_init() { ++test_sntp_starts; test_sntp_active = true; }
inline void esp_sntp_setoperatingmode(int) {}
inline void esp_sntp_setservername(int, const char *) {}

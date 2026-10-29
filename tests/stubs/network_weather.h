/** @brief Deterministic worker mailbox, not HTTP/TLS or a Wi-Fi simulation. */
#pragma once
#include "../../OTA-bot-Mochi/src/network/network_weather.h"
inline bool test_weather_available = true, test_weather_pending = false, test_weather_ready = false;
inline unsigned test_weather_jobs = 0;
inline WeatherJob test_weather_job;
inline WeatherReply test_weather_reply;

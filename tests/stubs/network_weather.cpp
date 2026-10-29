/** @brief Main-thread worker fixture: tests inject completed owned responses explicitly. */
#include "network_weather.h"
bool weather_worker_begin() { return test_weather_available; }
bool weather_worker_submit(const WeatherJob &job) {
	if (!test_weather_available || test_weather_pending) { return false; }
	test_weather_pending = true; test_weather_job = job; ++test_weather_jobs; return true;
}
bool weather_worker_take(WeatherReply &reply) {
	if (!test_weather_ready) { return false; }
	reply = test_weather_reply; test_weather_ready = test_weather_pending = false; return true;
}

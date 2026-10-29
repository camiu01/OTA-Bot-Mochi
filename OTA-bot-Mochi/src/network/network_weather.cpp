/**
 * @file network_weather.cpp
 * @brief One bounded HTTPS worker using the ESP32 certificate bundle, never insecure TLS.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "network_weather.h"
#include <esp_http_client.h>
#include <esp_crt_bundle.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {
QueueHandle_t requests = nullptr, replies = nullptr;
constexpr size_t BODY_LIMIT = 8192;
struct Body { char *data; size_t used = 0; uint32_t started; bool failed = false; };
/** @brief Bound both retained bytes and total response time, including slow-drip responses. */
esp_err_t receive(esp_http_client_event_t *event) {
	Body &body = *static_cast<Body *>(event->user_data);
	if (uint32_t(millis() - body.started) >= 20000) { body.failed = true; return ESP_FAIL; }
	if (event->event_id == HTTP_EVENT_ON_DATA) {
		if (event->data_len < 0 || size_t(event->data_len) > BODY_LIMIT - body.used) { body.failed = true; return ESP_FAIL; }
		memcpy(body.data + body.used, event->data, event->data_len); body.used += event->data_len;
		body.data[body.used] = 0;
	}
	return ESP_OK;
}
/** @brief Percent-encode user input as a query value, never as a URL or hostname. */
void query_value(const char *source, char *out) {
	for (const uint8_t *c = reinterpret_cast<const uint8_t *>(source); *c; ++c) {
		if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') ||
			(*c >= '0' && *c <= '9') || *c == '-' || *c == '_' || *c == '.') { *out++ = char(*c); }
		else { snprintf(out, 4, "%%%02X", unsigned(*c)); out += 3; }
	}
	*out = 0;
}
/** @brief Use only the two fixed documented HTTPS origins, rejecting redirects. */
void fetch(const WeatherJob &job, WeatherReply &reply) {
	char url[512] = {};
	if (job.search) {
		char encoded[193]; query_value(job.query, encoded);
		snprintf(url, sizeof(url), "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=5&language=%s&format=json",
			encoded, job.italian ? "it" : "en");
	} else {
		snprintf(url, sizeof(url), "https://api.open-meteo.com/v1/forecast?latitude=%.6f&longitude=%.6f"
			"&current=temperature_2m,weather_code&daily=temperature_2m_max,temperature_2m_min,weather_code"
			"&temperature_unit=celsius&timezone=auto&forecast_days=3", job.latitude / 1000000.0, job.longitude / 1000000.0);
	}
	char *buffer = static_cast<char *>(calloc(BODY_LIMIT + 1, 1));
	if (!buffer) { return; }
	Body body{buffer, 0, millis()};
	esp_http_client_config_t config{};
	config.url = url; config.timeout_ms = 8000; config.disable_auto_redirect = true;
	config.crt_bundle_attach = esp_crt_bundle_attach; config.event_handler = receive;
	config.user_data = &body; config.user_agent = "Mochi/1"; config.buffer_size = 1024;
	config.is_async = true;
	esp_http_client_handle_t client = esp_http_client_init(&config);
	if (client) {
		esp_http_client_set_header(client, "Accept", "application/json");
		esp_err_t result = ESP_ERR_HTTP_EAGAIN;
		while (result == ESP_ERR_HTTP_EAGAIN && !body.failed && uint32_t(millis() - body.started) < 20000) {
			result = esp_http_client_perform(client);
			if (result == ESP_ERR_HTTP_EAGAIN) { vTaskDelay(pdMS_TO_TICKS(10)); }
		}
		if (result == ESP_OK && !body.failed && esp_http_client_get_status_code(client) == 200) {
			network_weather_parse(buffer, body.used, reply);
		}
		esp_http_client_cleanup(client);
	}
	free(buffer);
}
/** @brief Block only this worker; transfer complete owned records back to the main task. */
void worker(void *) {
	WeatherJob job;
	for (;;) {
		if (xQueueReceive(requests, &job, portMAX_DELAY) != pdTRUE) { continue; }
		WeatherReply reply; reply.search = job.search; reply.revision = job.revision;
		fetch(job, reply);
		xQueueSend(replies, &reply, portMAX_DELAY);
	}
}
}
bool weather_worker_begin() {
	if (requests && replies) { return true; }
	requests = xQueueCreate(1, sizeof(WeatherJob)); replies = xQueueCreate(1, sizeof(WeatherReply));
	if (requests && replies && xTaskCreate(worker, "mochi-weather", 8192, nullptr, 1, nullptr) == pdPASS) { return true; }
	if (requests) { vQueueDelete(requests); }
	if (replies) { vQueueDelete(replies); }
	requests = replies = nullptr; return false;
}
bool weather_worker_submit(const WeatherJob &job) { return requests && xQueueSend(requests, &job, 0) == pdTRUE; }
bool weather_worker_take(WeatherReply &reply) { return replies && xQueueReceive(replies, &reply, 0) == pdTRUE; }

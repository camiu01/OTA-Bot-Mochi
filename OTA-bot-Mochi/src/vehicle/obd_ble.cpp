/**
 * @file obd_ble.cpp
 * @brief NimBLE central for BLE ELM327 dongles, isolated from display frames.
 *
 * All scanning, connecting, GATT discovery and request/response waits run in a
 * priority-1 worker. The notify callback only copies bounded fragments into a
 * queue. While the drive page is hidden the worker stops polling, and it drops
 * the link after a minute so the dongle does not keep the car's bus awake.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "obd_ble.h"
#include "elm327.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <cstring>

namespace {
struct Chunk { uint8_t length; uint8_t bytes[63]; };
QueueHandle_t chunks = nullptr;
NimBLEClient *client = nullptr;
NimBLERemoteCharacteristic *tx = nullptr;	/**< Dongle write characteristic. */
VehicleData state;
ElmSession session;

/** @brief Copy only bounded notification bytes; no GATT or drawing here. */
void received(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
	while (chunks && length) {
		Chunk chunk{};
		chunk.length = uint8_t(length > sizeof(chunk.bytes) ? sizeof(chunk.bytes) : length);
		memcpy(chunk.bytes, data, chunk.length);
		xQueueSend(chunks, &chunk, 0);
		data += chunk.length; length -= chunk.length;
	}
}

/** @brief Publish a status change with the current values. @param status Link state. */
void publish(ObdStatus status) { state.status = status; obd_publish(state); }

/** @brief Recognize common BLE OBD names case-insensitively. @param name Advertised name. @return Likely ELM327. */
bool obd_name(const std::string &name) {
	static const char *hints[] = {"OBD", "ELM", "VLINK", "V-LINK", "VGATE", "ICAR", "VEEPEAK", "KONNWEI", "OBDLINK"};
	std::string upper = name;
	for (char &c : upper) { c = char(toupper((unsigned char)c)); }
	for (const char *hint : hints) { if (upper.find(hint) != std::string::npos) { return true; } }
	return false;
}

/** @brief Serial-over-GATT services used by known dongles, tried before any other service. */
const char *const KNOWN[] = {"FFF0", "FFE0", "18F0", "E7810A71-73AE-499D-8C15-FAA9AEF0C3F2"};

/** @brief Pick a notify and a write characteristic in one service. @param service Remote service. @return Subscribed. */
bool attach(NimBLERemoteService *service) {
	NimBLERemoteCharacteristic *rx = nullptr, *write = nullptr;
	for (auto *chr : service->getCharacteristics(true)) {
		if (!rx && (chr->canNotify() || chr->canIndicate())) { rx = chr; }
		if (!write && (chr->canWrite() || chr->canWriteNoResponse())) { write = chr; }
	}
	if (!rx || !write || !rx->subscribe(rx->canNotify(), received)) { return false; }
	tx = write;
	return true;
}

/** @brief Find the dongle's UART-like service, known UUIDs first. @return Ready to talk. */
bool discover() {
	for (const char *uuid : KNOWN) {
		auto *service = client->getService(NimBLEUUID(uuid));
		if (service && attach(service)) { return true; }
	}
	for (auto *service : client->getServices(true)) {
		const NimBLEUUID &id = service->getUUID();
		if (id == NimBLEUUID(uint16_t(0x1800)) || id == NimBLEUUID(uint16_t(0x1801)) ||
			id == NimBLEUUID(uint16_t(0x180A))) { continue; }	// GAP, GATT, device info.
		if (attach(service)) { return true; }
	}
	return false;
}

/** @brief Scan four seconds and connect to the first likely dongle. @return Connected and discovered. */
bool connect() {
	publish(ObdStatus::Scanning);
	NimBLEScan *scan = NimBLEDevice::getScan();
	scan->setActiveScan(true);
	NimBLEScanResults results = scan->getResults(4000, false);
	NimBLEAddress address;
	bool found = false;
	for (int i = 0; i < results.getCount() && !found; ++i) {
		const NimBLEAdvertisedDevice *device = results.getDevice(i);
		bool service = false;
		for (const char *uuid : KNOWN) { service |= device->isAdvertisingService(NimBLEUUID(uuid)); }
		if (!obd_name(device->getName()) && !service) { continue; }
		address = device->getAddress();
		strncpy(state.name, device->getName().c_str(), sizeof(state.name) - 1);
		found = true;
	}
	scan->clearResults();
	if (!found) { return false; }
	publish(ObdStatus::Connecting);
	if (!client) { client = NimBLEDevice::createClient(); client->setConnectTimeout(5000); }
	if (!client->connect(address) || !discover()) { client->disconnect(); tx = nullptr; return false; }
	session.reset();
	publish(ObdStatus::Starting);
	return true;
}

/** @brief Send one command and wait for its prompt-terminated reply. @param buffer Reply. @return Prompt received. */
bool exchange(ElmBuffer &buffer) {
	Chunk chunk{};
	while (xQueueReceive(chunks, &chunk, 0) == pdTRUE) {}
	char line[12];
	snprintf(line, sizeof(line), "%s\r", session.command());
	buffer.clear();
	if (!tx->writeValue(reinterpret_cast<const uint8_t *>(line), strlen(line), tx->canWrite())) { return false; }
	uint32_t start = millis();
	while (uint32_t(millis() - start) < session.timeout_ms()) {
		if (xQueueReceive(chunks, &chunk, pdMS_TO_TICKS(50)) == pdTRUE && buffer.feed(chunk.bytes, chunk.length)) { return true; }
	}
	return false;
}

/** @brief Own the whole link lifecycle away from the main loop. */
void worker(void *) {
	ElmBuffer buffer;
	uint32_t hidden = 0;
	for (;;) {
		bool linked = client && client->isConnected() && tx;
		if (!obd_wanted()) {
			if (!hidden) { hidden = millis(); }
			if (linked && uint32_t(millis() - hidden) > 60000) { client->disconnect(); }
			if (!linked && state.status != ObdStatus::Idle) { state = VehicleData{}; publish(ObdStatus::Idle); }
			vTaskDelay(pdMS_TO_TICKS(500));
			continue;
		}
		hidden = 0;
		if (!linked) {
			tx = nullptr; state.valid = 0;
			if (!connect()) { publish(ObdStatus::Scanning); vTaskDelay(pdMS_TO_TICKS(6000)); }
			continue;
		}
		bool framed = exchange(buffer);
		ObdStatus status = session.reply(framed ? buffer.text : "", state);
		if (framed && status == ObdStatus::Live) { state.updated = millis(); }
		publish(status);
		if (status == ObdStatus::NoEcu) { vTaskDelay(pdMS_TO_TICKS(1000)); }
	}
}
}

bool obd_begin() {
	if (chunks) { return true; }
	chunks = xQueueCreate(16, sizeof(Chunk));
	if (!chunks) { return false; }
	(void)obd_snapshot();	// Create the shared lock on the main task.
	return xTaskCreatePinnedToCore(worker, "mochi-obd", 6144, nullptr, 1, nullptr, 0) == pdPASS;
}

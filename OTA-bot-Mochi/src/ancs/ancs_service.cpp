/**
 * @file ancs_service.cpp
 * @brief Consume iPhone ANCS and AMS on one dedicated NimBLE peripheral profile.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ancs_service.h"
#include "ancs_parser.h"
#include "ams_service.h"
#include "../ui/ui_state.h"
#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <cstring>

namespace {
constexpr char SERVICE[] = "7905F431-B5CE-4E99-A40F-4B1E122D00D0";
constexpr char SOURCE[] = "9FBF120D-6301-42D9-8C58-25E699A21DBD";
constexpr char CONTROL[] = "69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9";
constexpr char DATA[] = "22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB";
enum class Kind : uint8_t { Connected, Disconnected, Source, Data, Media, Commands, Services };
struct Event {
	Kind kind;
	uint16_t handle = 0;
	uint16_t length = 0;
	uint8_t bytes[520] = {};
};
QueueHandle_t events = nullptr;
StaticQueue_t queue_storage;
uint8_t queue_bytes[sizeof(Event) * 8];
AncsParser parser;
std::string app_response;
bool app_pending = false;
uint32_t app_started = 0;

/** @brief Copy only bounded BLE callback data into the worker queue. */
void enqueue(Kind kind, uint16_t handle, const uint8_t *data, size_t length) {
	if (!events || length > sizeof(Event::bytes)) { return; }
	Event event{};
	event.kind = kind; event.handle = handle; event.length = uint16_t(length);
	if (length) { memcpy(event.bytes, data, length); }
	xQueueSend(events, &event, 0);
}

class ServerCallbacks : public NimBLEServerCallbacks {
public:
	/** @brief Queue discovery after iPhone connects. */
	void onConnect(NimBLEServer *, NimBLEConnInfo &info) override {
		ui_connection(true);
		enqueue(Kind::Connected, info.getConnHandle(), nullptr, 0);
	}
	/** @brief Drop pending phone data and resume pairing advertisements. */
	void onDisconnect(NimBLEServer *, NimBLEConnInfo &info, int) override {
		enqueue(Kind::Disconnected, info.getConnHandle(), nullptr, 0);
		ui_connection(false);
		NimBLEDevice::startAdvertising();
	}
};
ServerCallbacks callbacks;

/** @brief Retain raw GATT fragments without doing GATT work in callbacks. */
void source_notify(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
	enqueue(Kind::Source, 0, data, length);
}

/** @brief Retain raw GATT fragments without doing GATT work in callbacks. */
void data_notify(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
	enqueue(Kind::Data, 0, data, length);
}

/** @brief Queue media metadata without reading or writing GATT in the callback. */
void media_notify(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
	enqueue(Kind::Media, 0, data, length);
}
/** @brief Queue the active player's supported commands. */
void commands_notify(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
	enqueue(Kind::Commands, 0, data, length);
}
/** @brief Rediscover services when iOS publishes or removes AMS. */
void services_notify(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool) {
	enqueue(Kind::Services, 0, nullptr, 0);
}

/** @brief Request the four bounded fields used by the existing alert UI. */
bool request(NimBLERemoteCharacteristic *control, uint32_t uid) {
	uint8_t command[] = {0, uint8_t(uid), uint8_t(uid >> 8), uint8_t(uid >> 16),
		uint8_t(uid >> 24), 0, 1, 160, 0, 3, 0, 4, 5};
	return control->writeValue(command, sizeof(command), true);
}

/** @brief Ask iOS for a display name after notification attributes arrive. */
bool request_app(NimBLERemoteCharacteristic *control) {
	const std::string &id = parser.alert().app;
	if (id.empty() || id.find('\0') != std::string::npos) { return false; }
	std::string command(1, '\1');
	command += id;
	command.push_back('\0');
	command.push_back('\0');
	return control->writeValue(reinterpret_cast<const uint8_t *>(command.data()), command.size(), true);
}

/** @brief Decode the app display-name response, then keep the bundle ID as fallback. */
bool app_fragment(const uint8_t *data, size_t length) {
	if (app_response.size() + length > 256) { return true; }
	app_response.append(reinterpret_cast<const char *>(data), length);
	const std::string &id = parser.alert().app;
	size_t offset = id.size() + 2;
	if (app_response.size() < offset + 3) { return false; }
	if (uint8_t(app_response[0]) != 1 || app_response.compare(1, id.size(), id) != 0 ||
		app_response[id.size() + 1] != '\0' || uint8_t(app_response[offset]) != 0) { return true; }
	size_t size = uint8_t(app_response[offset + 1]) |
		size_t(uint8_t(app_response[offset + 2])) << 8;
	if (size > 64) { return true; }
	if (app_response.size() < offset + 3 + size) { return false; }
	parser.set_app(app_response.substr(offset + 3, size));
	return true;
}

/** @brief Discover only after bonding, on a dedicated low-priority task. */
NimBLERemoteCharacteristic *discover(NimBLEClient *client) {
	if (!client || !client->isConnected() || !client->secureConnection()) { return nullptr; }
	auto *service = client->getService(SERVICE);
	if (!service) { return nullptr; }
	auto *source = service->getCharacteristic(SOURCE);
	auto *data = service->getCharacteristic(DATA);
	auto *control = service->getCharacteristic(CONTROL);
	if (!source || !data || !control || !data->subscribe(true, data_notify) ||
		!source->subscribe(true, source_notify)) { return nullptr; }
	return control;
}

/** @brief Convert a complete ANCS response into the existing bounded UI path. */
void publish() {
	const AncsAlert &alert = parser.alert();
	Notification notification;
	notification.app = String(alert.app.c_str());
	notification.title = String(alert.title.c_str());
	notification.message = String(alert.message.c_str());
	if (alert.date.size() >= 13) {
		notification.time = String((alert.date.substr(9, 2) + ":" + alert.date.substr(11, 2)).c_str());
	}
	ui_notification(notification);
}

/** @brief Subscribe to iOS service changes on the paired connection. @param client iPhone. @return Subscription installed. */
bool service_changes(NimBLEClient *client) {
	auto *gatt = client->getService("1801");
	auto *changed = gatt ? gatt->getCharacteristic("2A05") : nullptr;
	return changed && changed->subscribe(false, services_notify);
}

/** @brief Retry services independently and consume media commands even under notification traffic. */
void service_tick(NimBLEClient *client, NimBLERemoteCharacteristic *&control,
	uint32_t &discovery_at, AmsService &ams, bool &changes) {
	if (!client || !client->isConnected()) { return; }
	ams.poll();
	if (uint32_t(millis() - discovery_at) < 5000) { return; }
	discovery_at = millis();
	if (!control) { control = discover(client); }
	if (!ams.ready()) { ams.discover(client, media_notify, commands_notify); }
	if (!changes) { changes = service_changes(client); }
}

/** @brief Consume queued ANCS packets independently of media traffic. @param event Packet. @param control ANCS control point. */
void notification_event(const Event &event, NimBLERemoteCharacteristic *control) {
	if (!control) { return; }
	if (event.kind == Kind::Source && control) {
		app_pending = false; app_response.clear();
		if (parser.event(event.bytes, event.length) && !request(control, parser.alert().uid)) {
			parser.reset();
		}
	} else if (event.kind == Kind::Data && app_pending) {
		if (app_fragment(event.bytes, event.length)) { app_pending = false; publish(); }
	} else if (event.kind == Kind::Data && parser.append(event.bytes, event.length)) {
		app_response.clear();
		app_pending = request_app(control);
		app_started = millis();
		if (!app_pending) { publish(); }
	}
}

/** @brief Own ANCS/AMS discovery, queued media writes, and parsing away from display frames. */
void worker(void *) {
	NimBLEClient *client = nullptr;
	NimBLERemoteCharacteristic *control = nullptr;
	uint32_t discovery_at = 0;
	AmsService ams;
	bool changes = false;
	Event event{};
	for (;;) {
		service_tick(client, control, discovery_at, ams, changes);
		if (xQueueReceive(events, &event, pdMS_TO_TICKS(200)) != pdTRUE) {
			if (app_pending && uint32_t(millis() - app_started) >= 2000) {
				app_pending = false; publish();
			}

			continue;
		}
		if (event.kind == Kind::Disconnected) {
			parser.reset(); app_response.clear(); app_pending = false;
			client = nullptr; control = nullptr; changes = false; ams.reset();
		} else if (event.kind == Kind::Connected) {
			parser.reset();
			vTaskDelay(pdMS_TO_TICKS(30));
			client = NimBLEDevice::getServer()->getClient(event.handle);
			control = discover(client);
			ams.reset(); ams.discover(client, media_notify, commands_notify);
			changes = client && service_changes(client);
			discovery_at = millis();
		} else if (event.kind == Kind::Services && client) {
			ams.reset(); control = nullptr; changes = false;
			parser.reset(); app_response.clear(); app_pending = false;
			client->deleteServices(); discovery_at = millis() - 5000;
		} else if (event.kind == Kind::Media || event.kind == Kind::Commands) {
			ams.receive(event.bytes, event.length, event.kind == Kind::Commands);
		} else { notification_event(event, control); }
	}
}
}

/** @brief Advertise ANCS solicitation without exposing the Chronos service. */
bool ancs_begin() {
	if (events) { return true; }
	events = xQueueCreateStatic(8, sizeof(Event), queue_bytes, &queue_storage);
	if (!events) { return false; }
	NimBLEDevice::init("OTA bot");
	NimBLEDevice::setSecurityAuth(true, false, true);
	NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
	auto *server = NimBLEDevice::createServer();
	server->setCallbacks(&callbacks, false);
	auto *advertising = NimBLEDevice::getAdvertising();
	NimBLEAdvertisementData advertisement;
	if (!advertisement.setFlags(0x06)) { return false; }
	NimBLEUUID uuid(SERVICE);
	uint8_t solicitation[18] = {17, 0x15};
	memcpy(solicitation + 2, uuid.getValue(), 16);
	if (!advertisement.addData(solicitation, sizeof(solicitation)) ||
		!advertising->setAdvertisementData(advertisement)) { return false; }
	NimBLEAdvertisementData scan;
	if (!scan.setName("OTA bot") || !advertising->setScanResponseData(scan)) { return false; }
	advertising->enableScanResponse(true);
	if (xTaskCreatePinnedToCore(worker, "ancs", 6144, nullptr, 1, nullptr, 0) != pdPASS) { return false; }
	return advertising->start();
}

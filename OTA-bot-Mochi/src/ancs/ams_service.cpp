/**
 * @file ams_service.cpp
 * @brief AMS subscriptions, bounded metadata, and remote commands outside BLE callbacks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ams_service.h"
#include <esp_log.h>
#include <vector>

namespace {
constexpr char SERVICE[] = "89D3502B-0F36-433A-8EF4-C502AD55F8DC";
constexpr char REMOTE[] = "9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2";
constexpr char UPDATE[] = "2F7CABCE-808D-411F-9A0C-BB92BA96C102";
constexpr char ATTRIBUTE[] = "C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7";
}

bool AmsService::discover(NimBLEClient *client, NimBLERemoteCharacteristic::notify_callback entity, NimBLERemoteCharacteristic::notify_callback commands) {
	if (!client || !client->isConnected() || !client->secureConnection()) { return false; }
	auto *service = client->getService(SERVICE);
	if (!service) { return false; }
	auto *remote = service->getCharacteristic(REMOTE);
	auto *update = service->getCharacteristic(UPDATE);
	if (!remote || !update || !remote->subscribe(true, commands) || !update->subscribe(true, entity)) { return false; }
	const uint8_t player[] = {0, 1, 2}, track[] = {2, 0, 2};
	if (!update->writeValue(player, sizeof(player), true) || !update->writeValue(track, sizeof(track), true)) {
		ESP_LOGW("ams", "Media subscriptions failed");
		return false;
	}
	_remote = remote;
	_attribute = service->getCharacteristic(ATTRIBUTE);
	_music.available = true;
	ui_music(_music);
	return true;
}

void AmsService::reset() {
	_remote = _attribute = nullptr;
	_music = UiMusic{};
	ui_music(_music);
	(void)ui_music_take();
}

void AmsService::receive(const uint8_t *data, size_t size, bool commands) {
	if (!ready() || !data) { return; }
	if (commands) {
		_music.commands = music_commands(data, size);
		ui_music(_music);
		return;
	}
	if (size < 3) { return; }
	if ((data[2] & 1) && _attribute && data[0] == 2 && (data[1] == 0 || data[1] == 2)) {
		uint8_t request[] = {data[0], data[1]};
		if (_attribute->writeValue(request, sizeof(request), true)) {
			auto value = _attribute->readValue();
			if (value.size()) {
				size_t count = value.size() > 160 ? 160 : value.size();
				std::vector<uint8_t> packet{data[0], data[1], 0};
				packet.insert(packet.end(), value.data(), value.data() + count);
				if (music_entity(_music, packet.data(), packet.size())) { ui_music(_music); return; }
			}
		} else { ESP_LOGW("ams", "Extended metadata request failed"); }
	}
	if (music_entity(_music, data, size)) { ui_music(_music); }
}

void AmsService::poll() {
	MusicAction action = ui_music_take();
	if (action == MusicAction::None || !ready()) { return; }
	uint8_t command = uint8_t(action);
	if (!(_music.commands & (1U << command))) { return; }
	if (!_remote->writeValue(&command, 1, true)) {
		ESP_LOGW("ams", "Media command failed; rediscovering service");
		reset();
	}
}

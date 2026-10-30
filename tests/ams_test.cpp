/**
 * @file ams_test.cpp
 * @brief Exercise the production AMS adapter through a controlled GATT transport.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ancs/ams_service.h"
#include <esp_log.h>
#include <cassert>
#include <iostream>

/** @brief Check absent services, subscriptions, truncation recovery, and failed writes. @return Success. */
int main() {
	assert(ui_state_init()); ui_connection(true);
	AmsService ams;
	NimBLEClient client;
	assert(!ams.discover(nullptr, nullptr, nullptr));
	assert(!ams.discover(&client, nullptr, nullptr));
	NimBLERemoteService service;
	NimBLERemoteCharacteristic remote, update, attribute;
	service.characteristics = {
		{"9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2", &remote},
		{"2F7CABCE-808D-411F-9A0C-BB92BA96C102", &update},
		{"C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7", &attribute}};
	client.service = &service; client.secure = false;
	assert(!ams.discover(&client, nullptr, nullptr)); client.secure = true;
	update.subscribe_ok = false;
	assert(!ams.discover(&client, nullptr, nullptr)); update.subscribe_ok = true;
	assert(ams.discover(&client, nullptr, nullptr));
	assert(update.writes == std::vector<std::vector<uint8_t>>({{0, 1, 2}, {2, 0, 2}}));
	assert(ui_snapshot().music.available && !ui_music_request(MusicAction::Next));
	const uint8_t supported[] = {2, 3, 4, 5, 6}; ams.receive(supported, sizeof(supported), true);
	const uint8_t short_title[] = {2, 2, 1, 'S'};
	attribute.value = {'S', 'o', 'n', 'g'};
	ams.receive(short_title, sizeof(short_title), false);
	assert(ui_snapshot().music.title == "Song" && attribute.writes.back() == std::vector<uint8_t>({2, 2}));
	assert(ui_music_request(MusicAction::Previous)); ams.poll();
	assert(remote.writes.back() == std::vector<uint8_t>({4}));
	const uint8_t volume[] = {0, 2, 0, '0', '.', '6'}; ams.receive(volume, sizeof(volume), false);
	assert(ui_snapshot().music.volume == 60);
	assert(ui_music_request(MusicAction::VolumeDown)); ams.poll();
	assert(remote.writes.back() == std::vector<uint8_t>({6}));
	assert(ui_music_request(MusicAction::VolumeUp)); ams.poll();
	assert(remote.writes.back() == std::vector<uint8_t>({5}));
	const uint8_t next_only[] = {3}; ams.receive(next_only, sizeof(next_only), true);
	assert(!ui_music_request(MusicAction::Toggle));
	assert(ui_music_request(MusicAction::Next)); remote.write_ok = false; ams.poll();
	assert(!ams.ready() && !ui_snapshot().music.available && test_log_warnings == 1);
	assert(ams.discover(&client, nullptr, nullptr));
	ams.receive(supported, sizeof(supported), true);
	attribute.write_ok = false; ams.receive(short_title, sizeof(short_title), false);
	assert(ui_snapshot().music.title == "S" && test_log_warnings == 2);
	assert(ui_music_request(MusicAction::Toggle)); ams.reset();
	assert(!ams.ready() && ui_music_take() == MusicAction::None);
	std::cout << "AMS subscriptions, metadata and commands passed\n";
	return 0;
}

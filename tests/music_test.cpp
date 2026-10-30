/**
 * @file music_test.cpp
 * @brief Media boundaries, capability gating, disconnects, and Chronos integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/music_protocol.h"
#include <cassert>
#include "../OTA-bot-Mochi/src/ui/music_transport.h"
#include <iostream>
#include <vector>

/** @brief Check malformed AMS packets and bounded metadata. */
void protocol_checks() {
	UiMusic music;
	assert(!music_entity(music, nullptr, 0));
	const uint8_t short_packet[] = {2, 2};
	assert(!music_entity(music, short_packet, sizeof(short_packet)));
	const uint8_t playing[] = {0, 1, 0, '1', ',', '1', ',', '0', '0'};
	assert(music_entity(music, playing, sizeof(playing)) && music.playing);
	const uint8_t malformed[] = {0, 1, 0, '7', ',', '1', ',', '0', '0'};
	assert(!music_entity(music, malformed, sizeof(malformed)) && music.playing);
	const uint8_t unknown[] = {2, 1, 0, 'a'};
	assert(!music_entity(music, unknown, sizeof(unknown)));
	const uint8_t reserved[] = {2, 2, 2, 'a'};
	assert(!music_entity(music, reserved, sizeof(reserved)));
	std::vector<uint8_t> title(1003, 't'); title[0] = 2; title[1] = 2; title[2] = 1;
	assert(music_entity(music, title.data(), title.size()) && music.title.length() == 160);
	title[1] = 0;
	assert(music_entity(music, title.data(), title.size()) && music.artist.length() == 96);
	const uint8_t clear[] = {2, 2, 0};
	assert(music_entity(music, clear, sizeof(clear)) && !music.title.length());
	const uint8_t ids[] = {0, 1, 2, 3, 4, 13, 255};
	assert(music_commands(ids, sizeof(ids)) == 28);
}

/** @brief Reject invalid phone volume and retain only supported volume buttons. */
void volume_checks() {
	UiMusic music;
	for (const auto &item : {std::pair<const char *, uint8_t>{"0", 0}, {"0.55", 55}, {"1", 100}}) {
		std::vector<uint8_t> packet{0, 2, 0};
		for (const char *p = item.first; *p; ++p) { packet.push_back(uint8_t(*p)); }
		assert(music_entity(music, packet.data(), packet.size()) && music.volume == item.second);
	}
	for (const char *bad : {"", "nan", "inf", "-0.1", "1.1", "0.5junk"}) {
		std::vector<uint8_t> packet{0, 2, 0};
		for (const char *p = bad; *p; ++p) { packet.push_back(uint8_t(*p)); }
		assert(!music_entity(music, packet.data(), packet.size()) && music.volume == 100);
	}
	const uint8_t commands[] = {2, 3, 4, 5, 6, 7};
	assert(music_commands(commands, sizeof(commands)) == 124);
}

/** @brief Check task handoff, capability changes, calls, and stale commands. */
void command_checks() {
	assert(ui_state_init());
	assert(!ui_music_request(MusicAction::Next));
	ui_connection(true);
	ui_music(UiMusic{"Song", "Artist", true, true, 28});
	assert(!ui_music_request(MusicAction::None));
	assert(ui_music_request(MusicAction::Next));
	assert(!ui_music_request(MusicAction::Previous));
	assert(ui_music_take() == MusicAction::Next && ui_music_take() == MusicAction::None);
	assert(ui_music_request(MusicAction::Previous));
	ui_music(UiMusic{"Song", "Artist", true, true, 4});
	assert(ui_music_take() == MusicAction::None);
	assert(!ui_music_request(MusicAction::Next));
	assert(ui_music_request(MusicAction::Toggle));
	ui_ringer("Caller", true);
	assert(ui_music_take() == MusicAction::None && !ui_music_request(MusicAction::Toggle));
	ui_ringer("", false);
	assert(ui_music_request(MusicAction::Toggle));
	ui_connection(false); ui_connection(true);
	assert(ui_music_take() == MusicAction::None && !ui_snapshot().music.available);
	ui_connection(false);
	ui_music(UiMusic{"Stale", "Old", true, true, 28});
	assert(!ui_snapshot().music.available);
}

/** @brief Verify Chronos snapshots do not generate automatic page events. */
void chronos_checks() {
	ChronosESP32 watch;
	ui_connection(true);
	watch.music = MusicInfo{1, String(std::string(400, 't')), String(std::string(200, 'a'))};
	UiPageEvents before = ui_page_events();
	ui_config(watch, CF_MUSIC, 0, 1);
	UiSnapshot snapshot = ui_snapshot();
	assert(snapshot.music.available && snapshot.music.playing && snapshot.music.commands == 124);
	assert(snapshot.music.title.length() == 160 && snapshot.music.artist.length() == 96);
	assert(ui_page_events().notifications == before.notifications && ui_page_events().route_starts == before.route_starts);
	watch.music.state = 0; ui_config(watch, CF_MUSIC, 0, 0);
	assert(!ui_snapshot().music.playing);
	const MusicAction actions[] = {MusicAction::Toggle, MusicAction::Next, MusicAction::Previous, MusicAction::VolumeUp, MusicAction::VolumeDown};
	const Control controls[] = {MUSIC_TOGGLE, MUSIC_NEXT, MUSIC_PREVIOUS, VOLUME_UP, VOLUME_DOWN};
	for (uint8_t i = 0; i < 5; ++i) {
		assert(ui_music_request(actions[i])); music_android_poll(watch);
		assert(watch.last_control == controls[i] && watch.controls == i + 1U);
	}
	ui_connection(false); music_android_poll(watch); assert(watch.controls == 5);
}

/** @brief Run native protocol and transport-handoff checks. @return Success. */
int main() {
	protocol_checks(); volume_checks(); command_checks(); chronos_checks();
	std::cout << "Music protocol and task handoff passed\n";
	return 0;
}

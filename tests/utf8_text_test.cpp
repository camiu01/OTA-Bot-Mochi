/**
 * @file utf8_text_test.cpp
 * @brief Exercise every cut of accented text, CJK and emoji plus malformed byte sequences.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/utf8_text.h"
#include "../OTA-bot-Mochi/src/ui/ui_state.h"
#include "../OTA-bot-Mochi/src/ui/music_protocol.h"
#include <cassert>
#include <string>

/** @brief Check whole code points at every possible byte budget. */
static void prefixes() {
	for (const std::string &point : {std::string("\xc3\xa8"), std::string("\xe6\x97\xa5"), std::string("\xf0\x9f\x98\x80")}) {
		std::string text = "abc" + point + "z";
		for (size_t limit = 0; limit <= text.size() + 1; ++limit) {
			size_t expected = limit < 3 ? limit : (limit < 3 + point.size() ? 3 :
				(limit < text.size() ? 3 + point.size() : text.size()));
			assert(utf8_prefix(text.data(), text.size(), limit) == expected);
		}
	}
	for (const std::string &bad : {std::string("x\x80"), std::string("x\xc0\xaf"), std::string("x\xed\xa0\x80"),
		std::string("x\xf4\x90\x80\x80"), std::string("x\xe2\x82"), std::string("x\xe2!x")}) {
		assert(utf8_prefix(bad.data(), bad.size(), bad.size()) == 1);
	}
	assert(utf8_prefix(nullptr, 10, 10) == 0);
}

/** @brief Check actual retained Chronos metadata across a multibyte boundary. @return Success. */
int main() {
	prefixes();
	assert(ui_state_init());
	ui_connection(true);
	UiMusic music;
	music.title = (std::string(159, 't') + "\xc3\xa8").c_str();
	music.artist = (std::string(95, 'a') + "\xf0\x9f\x98\x80").c_str();
	ui_music(music);
	UiSnapshot snapshot = ui_snapshot();
	assert(snapshot.music.title.length() == 159);
	assert(snapshot.music.artist.length() == 95);
	std::string metadata = std::string(159, 't') + "\xc3\xa8";
	std::string packet = std::string("\x02\x02\x00", 3) + metadata;
	assert(music_entity(music, reinterpret_cast<const uint8_t *>(packet.data()), packet.size()));
	assert(music.title.length() == 159);
}

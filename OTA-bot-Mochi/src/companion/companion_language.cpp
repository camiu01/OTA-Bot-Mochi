/** @file companion_language.cpp @brief User-confirmed language IDs; no guessed protocol locale numbers. */
#include "companion_language.h"
#include <Preferences.h>
namespace {
uint8_t bindings[2] = {255, 255};
}
bool companion_language_init() {
	bindings[0] = bindings[1] = 255;
	Preferences storage;
	if (!storage.begin("mochi-chronos", true)) { return false; }
	uint8_t bytes[4] = {};
	bool read = storage.getBytesLength("language") == 4 && storage.getBytes("language", bytes, 4) == 4;
	storage.end();
	if (!read || bytes[0] != 1 || bytes[3] != uint8_t(0xa5 ^ bytes[0] ^ bytes[1] ^ bytes[2]) ||
		(bytes[1] != 255 && bytes[1] == bytes[2])) { return false; }
	bindings[0] = bytes[1]; bindings[1] = bytes[2]; return true;
}
bool companion_language_bind(uint8_t id, uint8_t language) {
	if (id == 255 || language > 1) { return false; }
	uint8_t next[4] = {1, bindings[0], bindings[1], 0};
	if (next[2 - language] == id) { next[2 - language] = 255; }
	next[1 + language] = id;
	next[3] = uint8_t(0xa5 ^ next[0] ^ next[1] ^ next[2]);
	Preferences storage;
	if (!storage.begin("mochi-chronos", false)) { return false; }
	bool saved = storage.putBytes("language", next, 4) == 4;
	storage.end();
	if (saved) { bindings[0] = next[1]; bindings[1] = next[2]; }
	return saved;
}
uint8_t companion_language_get(uint8_t id) {
	if (id == 255) { return 255; }
	return id == bindings[0] ? 0 : (id == bindings[1] ? 1 : 255);
}

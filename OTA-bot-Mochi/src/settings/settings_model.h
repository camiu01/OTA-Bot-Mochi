/**
 * @file settings_model.h
 * @brief Validated, portable dashboard preferences and named choices.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <string.h>
#include "../eyes/eyes_types.h"
#include "../eyes/face_flow.h"
constexpr uint8_t SETTINGS_V4_BYTES = 12;	/**< Stored size of the retired version-four profile. */
constexpr uint8_t SETTINGS_LEGACY_BYTES = 13; /**< Stored size of version-five through version-eight profiles. */
constexpr uint8_t SETTINGS_BYTES = 16;		/**< Stored size of the current version-nine profile. */

struct CompanionSettings {
	uint8_t style = 1;
	uint8_t color = 0;
	uint8_t mood = 0;
	uint8_t brightness = 30;
	uint8_t night_brightness = 5;
	bool night = false;
	bool night_auto = false;
	uint8_t night_start = 22, night_end = 7;
	bool dashboard = true;
	bool shake = false;
	bool dim = false;
	bool random_faces = false;
	uint8_t face_flow = FACE_RETURN;
	bool imu_faces = false;
	bool route_large = true;
	bool sound = false;
	bool sound_faces = true, sound_timer = true, sound_alerts = true, sound_ambient = true;
	bool sound_night_mute = true;
	bool sound_sd = false;
	uint8_t sound_level = 10;
	uint8_t animation = 1;
	uint8_t language = 0;
	bool flipped = false;
	bool phone_ios = false;
};

/** @brief Name an eye shape. @param index Choice index. @return Stable USB/menu name. */
inline const char *settings_style_name(uint8_t index) {
	static const char *names[] = {"normal", "mochi", "bloub", "trapez"};
	return index < 4 ? names[index] : "unknown";
}

/** @brief Name an eye color. @param index Choice index. @return Stable USB/menu name. */
inline const char *settings_color_name(uint8_t index) {
	static const char *names[] = {"white", "mint", "cyan", "amber", "pink", "blue"};
	return index < 6 ? names[index] : "unknown";
}

/** @brief Name the selected expression. @param index Choice index. @return Stable USB/menu name. */
inline const char *settings_mood_name(uint8_t index) {
	return eyes_expression_name(static_cast<Expression>(index));
}
/** @brief Name a motion preset for stable USB commands. @param index Choice. @return Name. */
inline const char *settings_animation_name(uint8_t index) {
	static const char *names[] = {"calm", "dasai", "playful"};
	return index < 3 ? names[index] : "unknown";
}
/** @brief Name a locale. @param index Choice. @return ISO-like code. */
inline const char *settings_language_name(uint8_t index) { return index == 0 ? "en" : (index == 1 ? "it" : "unknown"); }

/** @brief Stable USB playback names. @param index Mode. @return Name. */
inline const char *settings_face_flow_name(uint8_t index) {
	static const char *names[] = {"return", "group5", "sequence", "continuous"};
	return index < FACE_FLOW_COUNT ? names[index] : "unknown";
}

/** @brief Validate persisted or incoming values. @param value Settings. @return All fields valid. */
inline bool settings_valid(const CompanionSettings &value) {
	return value.style < 4 && value.color < 6 && value.mood < EXPRESSION_COUNT &&
		value.brightness >= 5 && value.brightness <= 90 &&
		value.night_brightness >= 1 && value.night_brightness <= 20 &&
		value.sound_level >= 1 && value.sound_level <= 30 && value.animation < 3 && value.language < 2 &&
		value.face_flow < FACE_FLOW_COUNT && value.night_start < 24 && value.night_end < 24 &&
		value.night_start != value.night_end;
}

/**
 * @brief Encode a versioned profile without relying on struct padding.
 * @param value Valid settings.
	 * @param bytes Output sixteen-byte version-nine profile.
 */
inline void settings_encode(const CompanionSettings &value, uint8_t bytes[SETTINGS_BYTES]) {
	bytes[0] = 9;
	bytes[1] = value.style; bytes[2] = value.color; bytes[3] = value.mood;
	bytes[4] = value.brightness; bytes[5] = value.night_brightness;
	bytes[6] = value.night | (value.dashboard << 1) | (value.shake << 2) | (value.dim << 3);
	bytes[6] |= (value.random_faces << 4) | (value.imu_faces << 5);
	bytes[6] |= (value.route_large << 6) | (value.phone_ios << 7);
	bytes[7] = value.sound | (value.sound_faces << 1) | (value.sound_timer << 2) |
		(value.sound_alerts << 3) | (value.sound_ambient << 4) | (value.sound_night_mute << 5) | (value.sound_sd << 6);
	bytes[8] = value.sound_level; bytes[9] = value.animation; bytes[10] = value.language;
	bytes[11] = uint8_t(value.flipped) | (value.face_flow << 1);
	bytes[12] = value.night_auto; bytes[13] = value.night_start; bytes[14] = value.night_end;
	bytes[15] = 0xA5;
	for (uint8_t i = 0; i < 15; ++i) { bytes[15] ^= bytes[i]; }
}

/**
 * @brief Validate and decode a stored profile, leaving output untouched on failure.
 * @param bytes Input profile.
	 * @param length Actual buffer length: eight for v1-v3, twelve for v4, thirteen for v5-v8, sixteen for v9.
 * @param value Output settings.
 * @return Valid version, checksum, flags, and fields.
 */
inline bool settings_decode(const uint8_t *bytes, size_t length, CompanionSettings &value) {
	if (!bytes || (length != 8 && length != SETTINGS_V4_BYTES && length != SETTINGS_LEGACY_BYTES && length != SETTINGS_BYTES)) { return false; }
	if ((length == 8 && (bytes[0] < 1 || bytes[0] > 3)) ||
		(length == SETTINGS_V4_BYTES && bytes[0] != 4) ||
		(length == SETTINGS_LEGACY_BYTES && (bytes[0] < 5 || bytes[0] > 8)) ||
		(length == SETTINGS_BYTES && bytes[0] != 9)) { return false; }
	uint8_t checksum = 0xA5;
	for (size_t i = 0; i + 1 < length; ++i) { checksum ^= bytes[i]; }
	if (bytes[length - 1] != checksum) { return false; }
	if (bytes[6] & (bytes[0] == 1 ? 0xF0 : (bytes[0] == 2 ? 0xC0 : (bytes[0] >= 7 ? 0 : 0x80)))) { return false; }
	CompanionSettings next;
	next.style = bytes[1]; next.color = bytes[2]; next.mood = bytes[3];
	next.brightness = bytes[4]; next.night_brightness = bytes[5];
	next.night = bytes[6] & 1; next.dashboard = bytes[6] & 2;
	next.shake = bytes[6] & 4; next.dim = bytes[6] & 8;
	if (bytes[0] >= 2) { next.random_faces = bytes[6] & 16; next.imu_faces = bytes[6] & 32; }
	if (bytes[0] >= 3) { next.route_large = bytes[6] & 64; }
	if (bytes[0] >= 7) { next.phone_ios = bytes[6] & 128; }
	if (bytes[0] >= 4) {
		if (bytes[7] & (bytes[0] >= 8 ? 0x80 : 0xC0)) { return false; }
		next.sound = bytes[7] & 1; next.sound_faces = bytes[7] & 2;
		next.sound_timer = bytes[7] & 4; next.sound_alerts = bytes[7] & 8;
		next.sound_ambient = bytes[7] & 16; next.sound_night_mute = bytes[7] & 32;
		next.sound_sd = bytes[0] >= 8 && (bytes[7] & 64);
		next.sound_level = bytes[8]; next.animation = bytes[9]; next.language = bytes[10];
	}
	if (bytes[0] >= 5) {
		if (bytes[11] & (bytes[0] == 5 ? ~1U : ~7U)) { return false; }
		next.flipped = bytes[11] & 1;
		if (bytes[0] >= 6) { next.face_flow = bytes[11] >> 1; }
	}
	if (bytes[0] == 9) {
		if (bytes[12] > 1) { return false; }
		next.night_auto = bytes[12]; next.night_start = bytes[13]; next.night_end = bytes[14];
	}
	if (!settings_valid(next)) { return false; }
	value = next;
	return true;
}

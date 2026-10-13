/**
 * @file settings.cpp
 * @brief Versioned preferences with explicit persistence and safe color presets.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "settings.h"
#include "../eyes/eyes_api.h"
#include "../power/backlight.h"
#include "../ui/ui_theme.h"
#include <Preferences.h>

namespace {
Preferences storage;
CompanionSettings profile;
uint8_t saved[SETTINGS_BYTES] = {};
bool storage_ready = false;
bool has_saved = false;
uint32_t revision = 0;
}

/** @brief Read a validated NVS blob; malformed profiles fall back to defaults. @return Storage open. */
bool settings_init() {
	profile = CompanionSettings{};
	has_saved = false;
	storage_ready = storage.begin("mochi", false);
	uint8_t bytes[SETTINGS_BYTES] = {};
	size_t length = storage_ready ? storage.getBytesLength("profile") : 0;
	if ((length == 8 || length == sizeof(bytes)) &&
		storage.getBytes("profile", bytes, length) == length &&
		settings_decode(bytes, length, profile)) {
		memcpy(saved, bytes, sizeof(saved));
		has_saved = true;
	}
	++revision;
	return storage_ready;
}

/** @brief Read preferences from the main task. @return Current profile. */
const CompanionSettings &settings_get() { return profile; }

/** @brief Apply preset eye colors on black and the selected backlight profile. */
void settings_apply() {
	static const uint16_t colors[] = {0xFFFF, 0x6FBA, 0x07FF, 0xFD6C, 0xF9D6, 0x4D7F};
	ui_theme_color(colors[profile.color]);
	eyes_set_style(static_cast<EyeStyle>(profile.style));
	if (!profile.shake) { eyes_state().dizzyUntilMs = 0; }
	eyes_set_expression(static_cast<Expression>(profile.mood));
	eyes_set_eye_colors(0x0000, colors[profile.color], 0x0000, 0xFFFF);
	eyes_state().dirtyInit = false;
	backlight_configure(profile.night ? profile.night_brightness : profile.brightness, profile.dim);
}

/** @brief Update a validated profile in RAM. @param value Settings. @return Accepted values. */
bool settings_update(const CompanionSettings &value) {
	if (!settings_valid(value)) { return false; }
	uint8_t before[SETTINGS_BYTES], after[SETTINGS_BYTES];
	settings_encode(profile, before);
	settings_encode(value, after);
	if (memcmp(before, after, sizeof(before)) == 0) { return true; }
	profile = value;
	++revision;
	settings_apply();
	return true;
}

/** @brief Determine whether a save is needed. @return Active profile differs from saved profile. */
bool settings_dirty() {
	uint8_t bytes[SETTINGS_BYTES];
	settings_encode(profile, bytes);
	return !has_saved || memcmp(bytes, saved, sizeof(bytes)) != 0;
}

/** @brief Persist a complete profile only on explicit request. @return Successful save. */
bool settings_save() {
	if (!storage_ready) { return false; }
	if (!settings_dirty()) { return true; }
	uint8_t bytes[SETTINGS_BYTES];
	settings_encode(profile, bytes);
	if (storage.putBytes("profile", bytes, sizeof(bytes)) != sizeof(bytes)) { return false; }
	memcpy(saved, bytes, sizeof(saved));
	has_saved = true;
	++revision;
	return true;
}

/** @brief Report preferences changes to the renderer. @return Revision counter. */
uint32_t settings_revision() { return revision; }

/** @brief Restore defaults without silently overwriting the saved profile. */
void settings_defaults() { settings_update(CompanionSettings{}); }

/**
 * @file night_schedule_test.cpp
 * @brief Scheduled brightness/mute, manual override, legacy migration and explicit persistence.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/night_policy.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/power/backlight.h"
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include <Preferences.h>
#include <algorithm>
#include <cassert>
#include <iostream>

/** @brief Verify both interval shapes, exact edges and missing local time. */
void interval_checks() {
	for (uint8_t hour = 0; hour < 24; ++hour) {
		assert(night_active(false, true, 22, 7, hour, true) == (hour >= 22 || hour < 7));
		assert(night_active(false, true, 9, 17, hour, true) == (hour >= 9 && hour < 17));
		assert(!night_active(false, true, 22, 7, hour, false));
		assert(!night_active(false, false, 22, 7, hour, true));
		assert(night_active(true, true, 22, 7, hour, false));
	}
	assert(!night_active(false, true, 7, 7, 7, true));
	assert(!night_active(false, true, 24, 7, 0, true));
	assert(!night_active(false, true, 22, 7, 24, true));
}

/** @brief Automatic edges change brightness and mute without changing NVS or manual flags. */
void runtime_checks() {
	assert(settings_init()); eyes_init(nullptr);
	auto p = CompanionSettings{}; p.night_auto = p.sound = true; p.sound_ambient = false;
	assert(settings_update(p) && settings_save() && !settings_dirty());
	unsigned writes = test_flash_writes;
	assert(!settings_night());
	settings_clock(true, 22);
	assert(settings_night() && backlight_brightness() == p.night_brightness);
	assert(!sound_request(SoundCue::Startup, 1000) && sound_status() == SoundStatus::Night);
	settings_clock(true, 0); settings_clock(true, 6); assert(settings_night());
	settings_clock(true, 7); assert(!settings_night() && backlight_brightness() == p.brightness);
	assert(!settings_get().night && !settings_dirty() && test_flash_writes == writes);
	settings_clock(true, 23); settings_clock(false, 23); assert(!settings_night());
	p.night = true; assert(settings_update(p)); settings_clock(true, 12); assert(settings_night());
	p.night = false; p.night_auto = false; assert(settings_update(p));
	settings_clock(true, 23); assert(!settings_night());
	p.night_auto = true; p.night_start = 7; p.night_end = 7; assert(!settings_update(p));
}

/** @brief A thirteen-byte v8 record retains its choices while automatic night defaults off. */
void migration_checks() {
	CompanionSettings p; p.sound_sd = p.phone_ios = true;
	uint8_t current[SETTINGS_BYTES]; settings_encode(p, current);
	current[0] = 8; current[12] = 0xA5;
	for (uint8_t i = 0; i < 12; ++i) { current[12] ^= current[i]; }
	test_profile.assign(current, current + SETTINGS_LEGACY_BYTES);
	assert(settings_init());
	assert(settings_get().sound_sd && settings_get().phone_ios && !settings_get().night_auto);
	assert(settings_get().night_start == 22 && settings_get().night_end == 7);
	assert(settings_dirty() && settings_save() && test_profile.size() == 16 && test_profile[0] == 9);
	p = settings_get(); p.night_auto = true; p.night_start = 21; p.night_end = 8;
	assert(settings_update(p) && settings_save() && settings_init());
	assert(settings_get().night_auto && settings_get().night_start == 21 && settings_get().night_end == 8);
	assert(!settings_night());
}

/** @brief Seven display rows and USB setters edit the same bounded schedule without saving. */
void controls_checks() {
	settings_defaults(); Arduino_GFX gfx;
	unsigned writes = test_flash_writes;
	settings_menu_tap(210, 65);
	assert(settings_menu_scroll(1) && settings_menu_scroll(1));
	settings_menu_tap(20, 202); assert(settings_get().night_auto);
	assert(settings_menu_scroll(1)); settings_menu_tap(20, 202); assert(settings_get().night_start == 23);
	assert(settings_menu_scroll(1)); settings_menu_tap(20, 202); assert(settings_get().night_end == 8);
	assert(!settings_menu_scroll(1)); settings_menu_draw(&gfx);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "23:00") != gfx.text.end());
	assert(std::find(gfx.text.begin(), gfx.text.end(), "08:00") != gfx.text.end());
	assert(test_flash_writes == writes && settings_dirty());
	auto parsed = settings_parse("set night-start 0", settings_get());
	assert(parsed.action == UsbAction::Set && parsed.profile.night_start == 0);
	assert(settings_parse("set night-end 24", settings_get()).action != UsbAction::Set);
	assert(settings_parse("set night-auto on", settings_get()).action == UsbAction::Set);
}

/** @brief Run portable scheduling and real settings integration. @return Success. */
int main() {
	interval_checks(); runtime_checks(); migration_checks(); controls_checks();
	std::cout << "Night schedule edges, brightness/mute, controls and migrations passed\n";
	return 0;
}

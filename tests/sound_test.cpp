/**
 * @file sound_test.cpp
 * @brief Short notes, optional categories, priority, night mute, failures, and rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <Arduino.h>
#include <assert.h>
#include <iostream>
static UiPageEvents events;

/** @brief Disabled audio never claims a pin; short notes have gaps and strict rate limits. */
static void note_tests() {
	sound_init(0);
	sound_request(SoundCue::Tap, 0);
	sound_poll(10, events, false, false, false);
	assert(sound_status() == SoundStatus::Off && test_pwm_pins[BUZZER] == -1 && !test_gpio_writes[BUZZER]);
	auto p = settings_get(); p.sound = true; p.sound_ambient = false; settings_update(p);
	sound_poll(1000, events, false, false, false);
	assert(sound_status() == SoundStatus::Armed && test_pwm_pins[BUZZER] == -1);
	sound_request(SoundCue::Tap, 1000);
	sound_poll(1000, events, false, false, false);
	sound_poll(1010, events, false, false, false);
	assert(test_pwm[2].frequency == 2200 && test_pwm[2].duty == 102);
	sound_request(SoundCue::Tap, 1050);
	sound_poll(1060, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_poll(1090, events, false, false, false);
	assert(test_pwm[2].frequency == 2800 && test_pwm[2].duty == 102);
	sound_poll(1150, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_request(SoundCue::Tap, 1210);
	sound_poll(1220, events, false, false, false);
	assert(test_pwm[2].duty == 0);
}
/** @brief Higher-priority events preempt taps; disabling a category or night mode immediately silences it. */
static void mute_tests() {
	sound_request(SoundCue::Tap, 1300); sound_poll(1300, events, false, false, false);
	++events.notifications;
	sound_poll(1310, events, false, false, false); sound_poll(1320, events, false, false, false);
	assert(test_pwm[2].frequency == 2500 && test_pwm[2].duty > 0);
	sound_request(SoundCue::Mood, 1320);
	sound_poll(1340, events, false, false, false);
	assert(test_pwm[2].frequency == 2500);
	auto p = settings_get(); p.sound_alerts = false; settings_update(p);
	sound_poll(1341, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	p.night = true; settings_update(p);
	sound_poll(1400, events, false, false, false);
	assert(sound_status() == SoundStatus::Night && test_pwm_pins[BUZZER] == -1);
	++events.calls; events.call_active = true;
	sound_request(SoundCue::Tap, 1500); sound_poll(1510, events, false, false, false);
	p.night = false; settings_update(p);
	sound_poll(1600, events, false, false, false);
	assert(sound_status() == SoundStatus::Armed);
	events.call_active = false;
	sound_poll(1700, events, false, false, true); sound_poll(1710, events, false, false, true);
	assert(test_pwm[2].frequency == 1600 && test_pwm[2].duty > 0);
	p.sound_timer = false; settings_update(p);
	sound_poll(1711, events, false, false, true);
	assert(test_pwm[2].duty == 0);
}
/** @brief Backend errors are silent and retried only after a mute/re-enable. */
static void failure_tests() {
	sound_init(2000); test_pwm_fail = true;
	sound_request(SoundCue::Tap, 2000); sound_poll(2000, events, false, false, false);
	assert(sound_status() == SoundStatus::Error);
	test_pwm_fail = false; sound_request(SoundCue::Tap, 2300);
	sound_poll(2300, events, false, false, false);
	assert(sound_status() == SoundStatus::Error && test_pwm_pins[BUZZER] == -1);
	auto p = settings_get(); p.sound = false; settings_update(p);
	sound_poll(2310, events, false, false, false);
	p.sound = true; settings_update(p);
	sound_request(SoundCue::Tap, 2400); sound_poll(2400, events, false, false, false);
	sound_poll(2410, events, false, false, false);
	assert(sound_status() == SoundStatus::Ready && test_pwm[2].duty > 0);
}
/** @brief Ambient chirps stay off utility/route/call screens and notes expire correctly across rollover. */
static void context_tests() {
	sound_init(0); events = {};
	auto p = settings_get(); p.sound_ambient = true; settings_update(p);
	events.route_active = true;
	sound_poll(60000, events, true, false, false);
	assert(test_pwm_pins[BUZZER] == -1);
	events.route_active = false;
	sound_poll(120000, events, true, false, false); sound_poll(120010, events, true, false, false);
	assert(test_pwm[2].frequency == 1900 && test_pwm[2].duty == 51);
	events.route_active = true; sound_poll(120020, events, true, false, false);
	assert(test_pwm[2].duty == 0);
	p.sound_ambient = false; settings_update(p);
	sound_init(UINT32_MAX - 50); events = {};
	sound_request(SoundCue::Tap, UINT32_MAX - 50);
	sound_poll(UINT32_MAX - 50, events, false, false, false);
	sound_poll(49, events, false, false, false);
	assert(test_pwm[2].frequency == 2800 && test_pwm[2].duty > 0);
	sound_poll(100, events, false, false, false);
	assert(test_pwm[2].duty == 0);
}
/** @brief Run native sound regressions, never emit actual sound. @return Success. */
int main() {
	assert(settings_init());
	note_tests(); mute_tests(); failure_tests(); context_tests();
	std::cout << "Sound notes, priority, categories, night mute, failure handling, and rollover passed\n";
}

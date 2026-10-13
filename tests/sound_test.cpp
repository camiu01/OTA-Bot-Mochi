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
/** @brief Face chirps stay inside family pitch bands and follow the faces switch. */
static void face_tone_tests() {
	sound_init(0); events = {};
	auto p = settings_get(); p.sound = true; p.night = false; p.sound_faces = true; settings_update(p);
	sound_request_face(FaceTone::Soft, 3000);
	sound_poll(3000, events, false, false, false);
	sound_poll(3010, events, false, false, false);
	assert(test_pwm[2].frequency >= 1000 && test_pwm[2].frequency <= 1300);
	sound_poll(3110, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_poll(3130, events, false, false, false);
	assert(test_pwm[2].frequency >= 1400 && test_pwm[2].frequency <= 1700);
	sound_poll(3210, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	// Family bands differ: a flat blip stays low and does not rise.
	sound_request_face(FaceTone::Flat, 4000);
	sound_poll(4000, events, false, false, false);
	sound_poll(4010, events, false, false, false);
	assert(test_pwm[2].frequency >= 800 && test_pwm[2].frequency <= 1049);
	sound_poll(4070, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_poll(4090, events, false, false, false);
	assert(test_pwm[2].frequency >= 800 && test_pwm[2].frequency <= 1049);
	sound_poll(4160, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	// The faces switch silences every family without touching the master switch.
	uint32_t quiet = test_pwm[2].frequency;
	p.sound_faces = false; settings_update(p);
	sound_request_face(FaceTone::Bright, 5000);
	sound_poll(5000, events, false, false, false);
	sound_poll(5010, events, false, false, false);
	assert(test_pwm[2].frequency == quiet && test_pwm[2].duty == 0);
}
/** @brief Manual bench test ignores category switches, but never bypasses master/night mute. */
static void manual_test() {
	sound_init(0); events = {};
	settings_defaults();
	sound_request(SoundCue::Test, 0);
	sound_poll(10, events, false, false, false);
	assert(sound_status() == SoundStatus::Off && test_pwm_pins[BUZZER] == -1);
	auto p = settings_get(); p.sound = true; p.night = true;
	p.sound_faces = p.sound_timer = p.sound_alerts = p.sound_ambient = false;
	settings_update(p);
	sound_request(SoundCue::Test, 100);
	sound_poll(110, events, false, false, false);
	assert(sound_status() == SoundStatus::Night && test_pwm_pins[BUZZER] == -1);
	p.night = false; settings_update(p);
	sound_request(SoundCue::Test, 200);
	sound_poll(200, events, false, false, false);
	sound_poll(210, events, false, false, false);
	assert(test_pwm[2].frequency == 1800 && test_pwm[2].duty > 0);
	assert(strcmp(sound_status_name(), "pwm-ready") == 0);
	sound_poll(470, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_poll(550, events, false, false, false);
	assert(test_pwm[2].frequency == 2400 && test_pwm[2].duty > 0);
	sound_poll(800, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_request(SoundCue::Test, 900);
	sound_poll(910, events, false, false, false);
	assert(test_pwm[2].duty == 0);
	sound_request(SoundCue::Test, 2000);
	sound_poll(2000, events, false, false, false);
	sound_poll(2010, events, false, false, false);
	p.sound = false; settings_update(p);
	sound_poll(2020, events, false, false, false);
	assert(test_pwm_pins[BUZZER] == -1 && strcmp(sound_status_name(), "off") == 0);
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
	note_tests(); mute_tests(); failure_tests(); context_tests(); manual_test(); face_tone_tests();
	std::cout << "Sound notes, priority, categories, night mute, failure handling, and rollover passed\n";
}

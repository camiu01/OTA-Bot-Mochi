/**
 * @file sound.cpp
 * @brief Bounded short sound sequences, no blocking delays or payload logging.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "sound.h"
#include "buzzer_output.h"
#include "../settings/settings.h"
#include <Arduino.h>

namespace {
struct Note { uint16_t hz, ms; };
struct Sequence { const Note *notes; uint8_t count; };
const Note tap[] = {{2200, 55}, {0, 25}, {2800, 65}};
const Note mood[] = {{1700, 50}, {2300, 80}};
const Note ambient[] = {{1900, 40}, {2400, 45}, {0, 20}, {2800, 45}};
const Note notification[] = {{2500, 75}, {0, 40}, {3000, 75}};
const Note call[] = {{2200, 100}, {0, 80}, {2200, 100}};
const Note rest[] = {{2400, 90}, {0, 45}, {2000, 110}};
const Note done[] = {{1600, 90}, {0, 50}, {2000, 90}, {0, 50}, {2600, 140}};
const Note test[] = {{1800, 250}, {0, 80}, {2400, 250}};
SoundCue queued = SoundCue::Count, active = SoundCue::Count;
uint32_t queued_at = 0, started = 0, ambient_since = 0, ambient_delay = 60000;
uint32_t last[uint8_t(SoundCue::Count)] = {};
bool seen[uint8_t(SoundCue::Count)] = {}, attached = false, failed = false, last_rest = false, last_done = false;
uint32_t notifications = 0, calls = 0;

/** @brief Select immutable notes. @param cue Cue. @return Sequence. */
Sequence sequence(SoundCue cue) {
	switch (cue) {
		case SoundCue::Ambient: return {ambient, 4};
		case SoundCue::Mood: return {mood, 2};
		case SoundCue::Tap: return {tap, 3};
		case SoundCue::Notification: return {notification, 3};
		case SoundCue::Call: return {call, 3};
		case SoundCue::Rest: return {rest, 3};
		case SoundCue::Done: return {done, 5};
		case SoundCue::Test: return {test, 3};
		default: return {nullptr, 0};
	}
}
/** @brief Check latest switches, including night mute. @param cue Cue. @return Allowed. */
bool allowed(SoundCue cue) {
	const auto &p = settings_get();
	if (!p.sound || (p.night && p.sound_night_mute)) { return false; }
	if (cue == SoundCue::Test) { return true; }
	if (cue == SoundCue::Ambient) { return p.sound_ambient; }
	if (cue == SoundCue::Tap || cue == SoundCue::Mood) { return p.sound_faces; }
	if (cue == SoundCue::Notification || cue == SoundCue::Call) { return p.sound_alerts; }
	return (cue == SoundCue::Rest || cue == SoundCue::Done) && p.sound_timer;
}
/** @brief Clear queued/active notes and release output. */
void stop() { queued = active = SoundCue::Count; buzzer_end(); attached = false; }
/** @brief Emit only the current note, skipping already expired notes after a slow frame. @param now Time. */
void play(uint32_t now) {
	Sequence s = sequence(active);
	uint32_t age = now - started;
	for (uint8_t i = 0; i < s.count; ++i) {
		if (age >= s.notes[i].ms) { age -= s.notes[i].ms; continue; }
		if (!attached) { attached = buzzer_begin(); if (!attached) { failed = true; stop(); return; } }
		uint8_t level = settings_get().sound_level;
		if (active == SoundCue::Ambient) { level = (level + 1) / 2; }
		uint32_t edge = age < 5 ? age : (s.notes[i].ms - age < 5 ? s.notes[i].ms - age : 5);
		level = uint32_t(level) * edge / 5;
		if (!buzzer_write(s.notes[i].hz, level)) { failed = true; stop(); }
		return;
	}
	active = SoundCue::Count;
	if (attached) { buzzer_write(0, 0); }
}
}

/** @brief Reset audio session; never initialize a pin merely because firmware booted. @param now Time. */
void sound_init(uint32_t now) {
	stop(); failed = false;
	for (uint8_t i = 0; i < uint8_t(SoundCue::Count); ++i) { seen[i] = false; last[i] = 0; }
	notifications = calls = 0; last_rest = last_done = false;
	ambient_since = now; ambient_delay = 60000;
}
/** @brief Request a bounded priority cue. @param cue Cue. @param now Time. */
void sound_request(SoundCue cue, uint32_t now) {
	if (cue >= SoundCue::Count || !allowed(cue) || failed) { return; }
	uint8_t index = uint8_t(cue);
	uint32_t cooldown = cue == SoundCue::Ambient ? 60000 : (cue == SoundCue::Tap ? 250 : 1500);
	if (seen[index] && uint32_t(now - last[index]) < cooldown) { return; }
	if ((active != SoundCue::Count && cue < active) || (queued != SoundCue::Count && cue < queued)) { return; }
	seen[index] = true; last[index] = now;
	queued = cue; queued_at = now;
}
/** @brief Service coherent events, keeping spontaneous sounds on idle eyes only. @param now Time. @param events Counters. @param eyesVisible Eyes context. @param resting Rest phase. @param done Timer end. */
void sound_poll(uint32_t now, const UiPageEvents &events, bool eyesVisible, bool resting, bool done) {
	if (events.notifications != notifications) { notifications = events.notifications; sound_request(SoundCue::Notification, now); }
	if (events.calls != calls) { calls = events.calls; if (events.call_active) { sound_request(SoundCue::Call, now); } }
	if (done && !last_done) { sound_request(SoundCue::Done, now); }
	else if (resting && !last_rest) { sound_request(SoundCue::Rest, now); }
	last_done = done; last_rest = resting;
	const auto &p = settings_get();
	if (!p.sound || (p.night && p.sound_night_mute)) { stop(); failed = false; ambient_since = now; return; }
	bool ambient_blocked = !eyesVisible || events.route_active || events.call_active;
	if (active == SoundCue::Ambient && ambient_blocked) { active = SoundCue::Count; if (attached) { buzzer_write(0, 0); } }
	if ((queued == SoundCue::Ambient && ambient_blocked) || (queued == SoundCue::Call && !events.call_active)) { queued = SoundCue::Count; }
	if (active != SoundCue::Count && (!allowed(active) || (active == SoundCue::Call && !events.call_active))) { active = SoundCue::Count; if (attached) { buzzer_write(0, 0); } }
	if (queued != SoundCue::Count && (!allowed(queued) || uint32_t(now - queued_at) > 700)) { queued = SoundCue::Count; }
	if (!eyesVisible || events.route_active || events.call_active) { ambient_since = now; }
	if (eyesVisible && !events.route_active && !events.call_active && uint32_t(now - ambient_since) >= ambient_delay) {
		sound_request(SoundCue::Ambient, now);
		ambient_since = now; ambient_delay = 60000 + uint32_t(random(60001L));
	}
	if (queued != SoundCue::Count) { active = queued; queued = SoundCue::Count; started = now; }
	if (active != SoundCue::Count) { play(now); }
}
/** @brief Status reflects configuration and PWM allocation, not measured sound. @return Status. */
SoundStatus sound_status() {
	const auto &p = settings_get();
	if (!p.sound) { return SoundStatus::Off; }
	if (p.night && p.sound_night_mute) { return SoundStatus::Night; }
	return failed ? SoundStatus::Error : (attached ? SoundStatus::Ready : SoundStatus::Armed);
}

const char *sound_status_name() {
	static const char *names[] = {"off", "night-mute", "armed", "pwm-ready", "pwm-error"};
	return names[uint8_t(sound_status())];
}

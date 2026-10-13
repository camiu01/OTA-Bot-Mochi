/**
 * @file usb_commands.cpp
 * @brief Strict named choices and bounded serial command processing.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "usb_commands.h"
#include "settings.h"
#include "../ui/view_state.h"
#include "../ui/ui_state.h"
#include "../ui/weather_sync.h"
#include "../sound/sound.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

namespace {
char line[96] = {};
uint8_t used = 0;
bool overflow = false;

/**
 * @brief Find a named preset.
 * @param name Choice text.
 * @param count Number of choices.
 * @param label Name lookup function.
 * @return Choice index, or -1.
 */
int choice(const char *name, uint8_t count, const char *(*label)(uint8_t)) {
	for (uint8_t i = 0; i < count; ++i) { if (strcmp(name, label(i)) == 0) { return i; } }
	return -1;
}

/** @brief Parse an on/off value. @param text Input. @param value Output. @return Valid toggle. */
bool toggle(const char *text, bool &value) {
	if (strcmp(text, "on") == 0) { value = true; return true; }
	if (strcmp(text, "off") == 0) { value = false; return true; }
	return false;
}

/** @brief Assign named choices. @param field Field. @param value Value. @param profile Candidate. @return Accepted. */
bool assign_choice(const char *field, const char *value, CompanionSettings &profile) {
	int index = -1;
	if (strcmp(field, "style") == 0) {
		index = choice(value, 4, settings_style_name);
		if (index >= 0) { profile.style = index; return true; }
	} else if (strcmp(field, "color") == 0) {
		index = choice(value, 6, settings_color_name);
		if (index >= 0) { profile.color = index; return true; }
	} else if (strcmp(field, "mood") == 0) {
		index = choice(value, EXPRESSION_COUNT, settings_mood_name);
		if (index >= 0) { profile.mood = index; return true; }
	} else if (strcmp(field, "animation") == 0) {
		index = choice(value, 3, settings_animation_name);
		if (index >= 0) { profile.animation = index; return true; }
	} else if (strcmp(field, "language") == 0) {
		index = choice(value, 2, settings_language_name);
		if (index >= 0) { profile.language = index; return true; }
	} else if (strcmp(field, "route") == 0) {
		if (strcmp(value, "large") == 0) { profile.route_large = true; return true; }
		if (strcmp(value, "details") == 0) { profile.route_large = false; return true; }
	}
	return false;
}

/** @brief Assign a toggle or numeric level. @param field Field. @param value Value. @param profile Candidate. @return Valid. */
bool assign(const char *field, const char *value, CompanionSettings &profile) {
	if (assign_choice(field, value, profile)) { return true; }
	if (strcmp(field, "brightness") == 0 || strcmp(field, "night-brightness") == 0 || strcmp(field, "sound-level") == 0) {
		char *end;
		long number = strtol(value, &end, 10);
		if (!*value || *end || number < 1 || number > 90) { return false; }
		if (strcmp(field, "brightness") == 0) { profile.brightness = number; }
		else if (strcmp(field, "night-brightness") == 0) { profile.night_brightness = number; }
		else { profile.sound_level = number; }
		return settings_valid(profile);
	} else if (strcmp(field, "night") == 0) { return toggle(value, profile.night); }
	else if (strcmp(field, "dashboard") == 0) { return toggle(value, profile.dashboard); }
	else if (strcmp(field, "shake") == 0) { return toggle(value, profile.shake); }
	else if (strcmp(field, "dim") == 0) { return toggle(value, profile.dim); }
	else if (strcmp(field, "random") == 0) { return toggle(value, profile.random_faces); }
	else if (strcmp(field, "imu-face") == 0) { return toggle(value, profile.imu_faces); }
	else if (strcmp(field, "sound") == 0) { return toggle(value, profile.sound); }
	else if (strcmp(field, "sound-faces") == 0) { return toggle(value, profile.sound_faces); }
	else if (strcmp(field, "sound-timer") == 0) { return toggle(value, profile.sound_timer); }
	else if (strcmp(field, "sound-alerts") == 0) { return toggle(value, profile.sound_alerts); }
	else if (strcmp(field, "sound-ambient") == 0) { return toggle(value, profile.sound_ambient); }
	else if (strcmp(field, "sound-night-mute") == 0) { return toggle(value, profile.sound_night_mute); }
	return false;
}

/** @brief Report active, non-sensitive settings over USB. */
void show() {
	const CompanionSettings &p = settings_get();
	Serial.printf("style=%s color=%s mood=%s brightness=%u night-brightness=%u\n",
		settings_style_name(p.style), settings_color_name(p.color),
		settings_mood_name(p.mood), p.brightness, p.night_brightness);
	Serial.printf("dashboard=%s night=%s shake=%s dim=%s unsaved=%s\n",
		p.dashboard ? "on" : "off", p.night ? "on" : "off", p.shake ? "on" : "off",
		p.dim ? "on" : "off", settings_dirty() ? "yes" : "no");
	Serial.printf("random=%s imu-face=%s\n", p.random_faces ? "on" : "off", p.imu_faces ? "on" : "off");
	Serial.printf("route-default=%s route-current=%s\n", p.route_large ? "large" : "details",
		view_route_large(p.route_large) ? "large" : "details");
	Serial.printf("animation=%s language=%s sound=%s sound-level=%u night-mute=%s\n",
		settings_animation_name(p.animation), settings_language_name(p.language),
		p.sound ? "on" : "off", p.sound_level, p.sound_night_mute ? "on" : "off");
	Serial.printf("sound-faces=%s sound-timer=%s sound-alerts=%s sound-ambient=%s\n",
		p.sound_faces ? "on" : "off", p.sound_timer ? "on" : "off",
		p.sound_alerts ? "on" : "off", p.sound_ambient ? "on" : "off");
	Serial.printf("sound-status=%s gpio=%u\n", sound_status_name(), unsigned(BUZZER));
}

/** @brief Print every accepted mood name for the USB vocabulary. */
void mood_names() {
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
		Serial.printf("%s%s", settings_mood_name(i), i + 1 < EXPRESSION_COUNT ? " " : "\n");
	}
}

/** @brief Explain strict USB vocabulary, unchanged by display locale. */
void help() {
	Serial.println("show | save | defaults | mood names | set <field> <value>");
	Serial.println("style normal/mochi/bloub/trapez; color white/mint/cyan/amber/pink/blue");
	Serial.println("mood <name>: any expression from neutral to cat; brightness 5..90; night-brightness 1..20");
	Serial.println("dashboard/night/shake/dim/random/imu-face on/off. Use controls only while parked.");
	Serial.println("set route large/details + save: default; route large/details/default: session only");
	Serial.println("animation calm/dasai/playful; language en/it; sound-level 1..30");
	Serial.println("sound/sound-faces/sound-timer/sound-alerts/sound-ambient/sound-night-mute on/off");
	Serial.println("weather: packet counters only; weather sync: request app synchronization");
	Serial.println("sound: mute/PWM status; sound test: two bench tones (master/night mute respected)");
}
/** @brief Execute a parsed command with explicit success/failure feedback. @param command Action. */
void execute(const UsbCommand &command) {
	switch (command.action) {
		case UsbAction::Help: help(); break;
		case UsbAction::Show: show(); break;
		case UsbAction::Set:
			Serial.println(settings_update(command.profile) ? "OK, changed in RAM; use save to persist" : "ERROR");
			break;
		case UsbAction::Save: Serial.println(settings_save() ? "OK, saved" : "ERROR, save failed"); break;
		case UsbAction::Defaults: settings_defaults(); Serial.println("Defaults applied; use save to persist"); break;
		case UsbAction::MoodNames: mood_names(); break;
		case UsbAction::RouteView:
			view_route_temporary(command.route_mode);
			Serial.println("OK, temporary route view; saved default unchanged");
			break;
		case UsbAction::WeatherStatus: {
			UiSnapshot data = ui_snapshot();
			Serial.printf("weather: current-packets=%lu forecast-packets=%lu city-packets=%lu items=%u available=%s\n",
				(unsigned long)data.weather_packets, (unsigned long)data.forecast_packets,
				(unsigned long)data.city_packets, data.weather_count, data.has_weather ? "yes" : "no");
			break;
		}
		case UsbAction::WeatherSync: weather_request(); Serial.println("App sync queued; requires connection/subscription"); break;
		case UsbAction::SoundStatus:
			Serial.printf("sound-status=%s gpio=%u level=%u\n", sound_status_name(), unsigned(BUZZER), settings_get().sound_level);
			break;
		case UsbAction::SoundTest: {
			SoundStatus status = sound_status();
			if (status == SoundStatus::Off || status == SoundStatus::Night || status == SoundStatus::Error) {
				Serial.printf("Sound test blocked: %s\n", sound_status_name());
			} else {
				sound_request(SoundCue::Test, millis());
				Serial.println("Sound test requested; type sound after one second for PWM status");
			}
			break;
		}
		default: Serial.println("ERROR, invalid command; type help"); break;
	}
}
}

/**
 * @brief Parse a strict command, normalizing ASCII case.
 * @param input Source command.
 * @param current Active profile.
 * @return Validated candidate or invalid action.
 */
UsbCommand settings_parse(const char *input, const CompanionSettings &current) {
	UsbCommand result;
	result.profile = current;
	if (!input || strlen(input) >= 96) { return result; }
	char text[96], verb[16], field[24], value[24], extra[2];
	for (size_t i = 0; i <= strlen(input); ++i) { text[i] = tolower((unsigned char)input[i]); }
	int tokens = sscanf(text, "%15s %23s %23s %1s", verb, field, value, extra);
	if (tokens == 1) {
		if (strcmp(verb, "help") == 0) { result.action = UsbAction::Help; }
		if (strcmp(verb, "show") == 0) { result.action = UsbAction::Show; }
		if (strcmp(verb, "save") == 0) { result.action = UsbAction::Save; }
		if (strcmp(verb, "defaults") == 0) { result.action = UsbAction::Defaults; }
		if (strcmp(verb, "weather") == 0) { result.action = UsbAction::WeatherStatus; }
		if (strcmp(verb, "sound") == 0) { result.action = UsbAction::SoundStatus; }
	} else if (tokens == 3 && strcmp(verb, "set") == 0 &&
		assign(field, value, result.profile) && settings_valid(result.profile)) {
		result.action = UsbAction::Set;
	} else if (tokens == 2 && strcmp(verb, "route") == 0) {
		if (strcmp(field, "large") == 0) { result.action = UsbAction::RouteView; result.route_mode = 1; }
		if (strcmp(field, "details") == 0) { result.action = UsbAction::RouteView; result.route_mode = 0; }
		if (strcmp(field, "default") == 0) { result.action = UsbAction::RouteView; result.route_mode = -1; }
	} else if (tokens == 2 && strcmp(verb, "mood") == 0 && strcmp(field, "names") == 0) {
		result.action = UsbAction::MoodNames;
	} else if (tokens == 2 && strcmp(verb, "weather") == 0 && strcmp(field, "sync") == 0) {
		result.action = UsbAction::WeatherSync;
	} else if (tokens == 2 && strcmp(verb, "sound") == 0 && strcmp(field, "test") == 0) {
		result.action = UsbAction::SoundTest;
	}
	return result;
}

/** @brief Poll at most 32 bytes per loop, rejecting oversized lines until their terminator. */
void settings_usb_poll() {
	for (uint8_t count = 0; count < 32 && Serial.available(); ++count) {
		int c = Serial.read();
		if (c == '\r' || c == '\n') {
			if (overflow) { Serial.println("ERROR, command too long"); }
			else if (used) { line[used] = '\0'; execute(settings_parse(line, settings_get())); }
			used = 0; overflow = false;
		} else if (!overflow) {
			if ((c < 32 && c != '\t') || size_t(used) + 1 >= sizeof(line)) { overflow = true; }
			else { line[used++] = char(c); }
		}
	}
}

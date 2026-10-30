/**
 * @file settings_web.h
 * @brief Validated named-field form API sharing the display's profile and actions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_web.h"
#include "../eyes/eyes_api.h"
#include "../eyes/imu_orientation.h"
#include "../sound/sound.h"

namespace settings_web {
struct Field {
	const char *key;
	Text label;
	uint8_t section, minimum, maximum;
	int8_t choices;
	uint8_t CompanionSettings::*number;
	bool CompanionSettings::*flag;
};
static const Field fields[] = {
	{"style", Text::EyesLabel, 0, 0, 3, 0, &CompanionSettings::style, nullptr},
	{"color", Text::ColorLabel, 0, 0, 5, 1, &CompanionSettings::color, nullptr},
	{"mood", Text::MoodLabel, 0, 0, EXPRESSION_COUNT - 1, 2, &CompanionSettings::mood, nullptr},
	{"brightness", Text::DayLevel, 1, 5, 90, -1, &CompanionSettings::brightness, nullptr},
	{"night", Text::NightManual, 1, 0, 1, -1, nullptr, &CompanionSettings::night},
	{"night_brightness", Text::NightLevel, 1, 1, 20, -1, &CompanionSettings::night_brightness, nullptr},
	{"night_auto", Text::NightAuto, 1, 0, 1, -1, nullptr, &CompanionSettings::night_auto},
	{"night_start", Text::NightStart, 1, 0, 23, -1, &CompanionSettings::night_start, nullptr},
	{"night_end", Text::NightEnd, 1, 0, 23, -1, &CompanionSettings::night_end, nullptr},
	{"dim", Text::IdleDim, 1, 0, 1, -1, nullptr, &CompanionSettings::dim},
	{"animation", Text::Animation, 2, 0, 2, 3, &CompanionSettings::animation, nullptr},
	{"random_faces", Text::RandomMood, 2, 0, 1, -1, nullptr, &CompanionSettings::random_faces},
	{"face_flow", Text::FaceFlow, 2, 0, FACE_FLOW_COUNT - 1, 4, &CompanionSettings::face_flow, nullptr},
	{"imu_faces", Text::ImuMood, 3, 0, 1, -1, nullptr, &CompanionSettings::imu_faces},
	{"shake", Text::Shake, 3, 0, 1, -1, nullptr, &CompanionSettings::shake},
	{"auto_rotate", Text::AutoRotate, 3, 0, 1, -1, nullptr, nullptr},
	{"flipped", Text::ScreenFlip, 3, 0, 1, -1, nullptr, &CompanionSettings::flipped},
	{"sound", Text::Sound, 4, 0, 1, -1, nullptr, &CompanionSettings::sound},
#if defined(MOCHI_BOARD_AMOLED_18)
	{"sound_sd", Text::SoundSd, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_sd},
#endif
	{"sound_level", Text::Level, 4, 1, 30, -1, &CompanionSettings::sound_level, nullptr},
	{"sound_night_mute", Text::NightMute, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_night_mute},
	{"sound_faces", Text::Faces, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_faces},
	{"sound_timer", Text::TimerLabel, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_timer},
	{"sound_alerts", Text::CallsAlerts, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_alerts},
	{"sound_ambient", Text::Ambient, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_ambient},
	{"sound_charging", Text::Charging, 4, 0, 1, -1, nullptr, &CompanionSettings::sound_charging},
	{"language", Text::Language, 5, 0, 1, 5, &CompanionSettings::language, nullptr},
	{"dashboard", Text::Dashboard, 5, 0, 1, -1, nullptr, &CompanionSettings::dashboard},
	{"route_large", Text::RouteDefault, 5, 0, 1, -1, nullptr, &CompanionSettings::route_large},
	{"phone_ios", Text::PhoneMode, 5, 0, 1, -1, nullptr, &CompanionSettings::phone_ios}
};
constexpr size_t COUNT = sizeof(fields) / sizeof(fields[0]);
/** @brief Include the session-only orientation toggle in optimistic concurrency. */
inline String revision() { return String(settings_revision()) + "-" + String(imu_orientation().enabled ? 1 : 0); }
/** @brief Read a live field without relying on padding or bool representation. */
inline uint8_t value(const Field &f, const CompanionSettings &p) {
	return f.number ? p.*(f.number) : (f.flag ? p.*(f.flag) : imu_orientation().enabled);
}
/** @brief Return localized schema and all live values, including enabled mood choices. */
inline void state(WebServer &s, int code = 200) {
	String json = String("{\"revision\":") + face_web::quoted(revision().c_str()) +
		",\"saved\":" + (settings_dirty() ? "false" : "true") + ",\"fields\":[";
	for (size_t i = 0; i < COUNT; ++i) {
		const Field &f = fields[i];
		if (i) { json += ','; }
		json += String("{\"key\":") + face_web::quoted(f.key) + ",\"label\":" + face_web::quoted(tr(f.label));
		json += String(",\"section\":") + face_web::quoted(tr_settings_tab(f.section));
		json += String(",\"value\":") + String(value(f, settings_get())) + ",\"min\":" + String(f.minimum) + ",\"max\":" + String(f.maximum);
		json += String(",\"toggle\":") + (f.number ? "false" : "true") + ",\"options\":[";
		if (f.choices >= 0) {
			for (uint8_t n = 0; n <= f.maximum; ++n) {
				if (n) { json += ','; }
				const char *label = f.choices == 5 ? (n ? "Italiano" : "English") : tr_choice(f.choices, n);
				json += String("{\"label\":") + face_web::quoted(label) + ",\"enabled\":" +
					(f.choices != 2 || face_filter().allows(Expression(n)) ? "true" : "false") + "}";
			}
		}
		json += "]}";
	}
	json += String("],\"calibration\":") + String(uint8_t(imu_orientation().status)) +
		",\"calibration_saved\":" + (imu_orientation().persisted && !imu_orientation().save_failed ? "true" : "false") +
		",\"sound\":" + face_web::quoted(sound_status_name()) + "}";
	face_web::headers(s); s.send(code, "application/json", json);
}
/** @brief Accept canonical bounded decimal values only. */
inline bool decimal(const String &text, uint8_t &out) {
	if (!text.length() || text.length() > 3 || (text.length() > 1 && text[0] == '0')) { return false; }
	unsigned n = 0;
	for (size_t i = 0; i < text.length(); ++i) { if (text[i] < '0' || text[i] > '9') { return false; } n = n * 10 + text[i] - '0'; }
	if (n > 255) { return false; } out = uint8_t(n); return true;
}
/** @brief Validate the entire form before committing any profile or orientation changes. */
inline void save(WebServer &s) {
	if (!(s.header("Content-Type") == "application/x-www-form-urlencoded") || s.args() != int(COUNT)) {
		s.send(400, "application/json", "{\"error\":\"Expected complete settings form\"}"); return;
	}
	if (!(s.header("X-Mochi-Settings-Revision") == revision())) { s.send(409, "application/json", "{\"error\":\"Reload settings\"}"); return; }
	auto next = settings_get(); bool rotate = imu_orientation().enabled;
	for (const Field &f : fields) {
		uint8_t n = 0;
		if (!s.hasArg(f.key) || !decimal(s.arg(f.key), n) || n < f.minimum || n > f.maximum) {
			s.send(400, "application/json", "{\"error\":\"Invalid setting\"}"); return;
		}
		if (f.number) { next.*(f.number) = n; } else if (f.flag) { next.*(f.flag) = n != 0; } else { rotate = n != 0; }
	}
	if (!settings_update(next)) { s.send(400, "application/json", "{\"error\":\"Selected face is excluded\"}"); return; }
	imu_orientation().enabled = rotate;
	state(s, settings_save() ? 200 : 507);
}
/** @brief Run existing display actions without bypassing mute or calibration availability. */
inline void action(WebServer &s) {
	if (!(s.header("Content-Type") == "application/x-www-form-urlencoded") || s.args() != 1) {
		s.send(400, "application/json", "{\"error\":\"Invalid action form\"}"); return;
	}
	if (s.arg("action") == "calibrate") {
		if (!eyes_imu_calibrate()) { imu_orientation().status = ImuCalibration::Failed; }
	} else if (s.arg("action") == "test") {
		sound_init(millis());
		sound_request(SoundCue::Test, millis());
	} else { s.send(400, "application/json", "{\"error\":\"Unknown action\"}"); return; }
	state(s);
}
}

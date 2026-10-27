/**
 * @file face_preset_test.cpp
 * @brief Catalog validity, compiled documentation, and unchanged public registrations.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../docs/examples/simple_face.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include <assert.h>
#include <set>
#include <string>
#include <iostream>
#include <limits>

/** @brief Test neutral values and each bounded parameter's rejection cases. */
static void parameter_tests() {
	assert(face_catalog_valid() && face_preset_valid(GUIDE_SIMPLE_FACE));
	assert(GUIDE_SIMPLE_FACE.right.openness == 0.7f);
	FacePreset p = GUIDE_SIMPLE_FACE;
	p.left.widthScale = 0; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.right.heightScale = 2.7f; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.left.openness = -0.1f; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.right.openness = 1.1f; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.gazeX = 1.1f; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.gazeY = -1.1f; assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.name = nullptr; assert(!face_preset_valid(p));
	assert(face_preset(NEUTRAL) && !face_preset(Expression(255)));
	EyeConfig cfg{};
	cfg.eyeW = 44; cfg.eyeH = 100; cfg.eyeGap = 42;
	EyeState st{};
	const auto l = eye_preset_measures(cfg, st, true, &GUIDE_SIMPLE_FACE);
	const auto r = eye_preset_measures(cfg, st, false, &GUIDE_SIMPLE_FACE);
	assert(l.w == 44 && l.h == 115 && r.w == 39 && r.h == 45);
	assert(l.ox == 3 && r.ox == 3 && l.oy == -1 && r.oy == -1);
	p = GUIDE_SIMPLE_FACE; p.left.widthScale = std::numeric_limits<float>::quiet_NaN();
	assert(!face_preset_valid(p));
	p = GUIDE_SIMPLE_FACE; p.gazeY = std::numeric_limits<float>::infinity();
	assert(!face_preset_valid(p));
	cfg.expression = SLEEPY_EYES;
	assert(eye_measures(cfg, st, true).h == 100);
	cfg.asym = 1;
	assert(eye_measures(cfg, st, true).h == 45 && eye_measures(cfg, st, false).h == 30);
	st.blinking = true;
	assert(eye_measures(cfg, st, true).h == 6 && eye_measures(cfg, st, false).h == 6);
}

/** @brief Every canonical name remains unique, selectable, and persistent by unchanged ID. */
static void public_tests() {
	std::set<std::string> names;
	for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) {
		const char *name = eyes_expression_name(Expression(id));
		assert(names.insert(name).second);
		assert(strcmp(face_preset(Expression(id))->name, name) == 0);
		assert(FACE_PRESETS[id].id == Expression(id));
		assert(eyes_set_expression_by_name(name) && eyes_config().expression == Expression(id));
		CompanionSettings p; p.mood = id;
		uint8_t bytes[SETTINGS_BYTES];
		settings_encode(p, bytes);
		CompanionSettings decoded;
		assert(settings_decode(bytes, sizeof(bytes), decoded) && decoded.mood == id);
		for (const char *prefix : {"mood ", "set mood "}) {
			auto result = settings_parse((std::string(prefix) + name).c_str(), p);
			assert(result.action == UsbAction::Set && result.profile.mood == id);
		}
	}
	assert(eyes_set_expression_by_name("SADDOWN") && eyes_config().expression == SAD_DOWN);
	assert(eyes_set_expression_by_name("sadup") && eyes_config().expression == SAD_UP);
	for (Expression id : {TIRED, FOCUSED, SLEEPY_EYES}) {
		Serial.clear(); Serial.feed(std::string("mood ") + eyes_expression_name(id) + "\n");
		while (Serial.available()) { settings_usb_poll(); }
		assert(settings_get().mood == id && eyes_config().expression == id);
	}
}

/** @brief Run the preset and public compatibility checks. */
int main() {
	assert(settings_init());
	eyes_init(nullptr);
	parameter_tests();
	public_tests();
	std::cout << "Face catalog, compiled guide, names, aliases, USB and settings passed\n";
}

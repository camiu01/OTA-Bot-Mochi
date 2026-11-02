/**
 * @file eye_style_regressions.h
 * @brief Permanent style IDs, profile/USB integration and bounded mirrored silhouettes.
 */
#pragma once
#include "../OTA-bot-Mochi/src/eyes/face_support.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include <cassert>
#include <cstring>

/** @brief New styles round-trip through unchanged v10 profiles and canonical USB names. */
static void signature_profile_tests() {
	static_assert(NORMAL == 0 && MOCHI == 1 && BLOUB == 2 && TRAPEZ == 3, "Preserve saved style IDs");
	auto profile = settings_get();
	for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
		profile.style = style;
		assert(settings_valid(profile));
		uint8_t bytes[SETTINGS_BYTES]; settings_encode(profile, bytes);
		CompanionSettings decoded;
		assert(bytes[0] == 10 && settings_decode(bytes, sizeof(bytes), decoded) && decoded.style == style);
		std::string command = std::string("set style ") + settings_style_name(style);
		auto parsed = settings_parse(command.c_str(), settings_get());
		assert(parsed.action != UsbAction::Invalid && parsed.profile.style == style);
		assert(std::strcmp(tr_choice(0, style), "?") != 0);
	}
	profile.style = EYE_STYLE_COUNT;
	assert(!settings_valid(profile));
	assert(settings_parse("set style invalid", settings_get()).action == UsbAction::Invalid);
}

/** @brief Style raster and morph envelopes agree, including odd sizes and blink-height boxes. */
static void signature_span_tests(EyeStyle style) {
	Arduino_GFX gfx(120, 140);
	EyeConfig cfg = eyes_config(); cfg.style = style; cfg.expression = NEUTRAL;
	cfg.fgColor = 0xFFFF; cfg.bgColor = 0;
	for (int16_t height : {6, 27, 54, 95}) {
		for (bool left_eye : {true, false}) {
			EyeBox box = {12, 10, 83, height, 0, left_eye};
			gfx.fillScreen(0); eye_body(&gfx, cfg, box);
			int count = 0;
			for (int16_t row = 0; row < height; ++row) {
				int16_t left, right; eye_body_span(cfg, box, row, left, right);
				if (style != ANIME) {
					EyeBox mirror = box; mirror.left = !box.left;
					int16_t ml, mr; eye_body_span(cfg, mirror, row, ml, mr);
					if (right >= left) { assert(left == box.w - 1 - mr && right == box.w - 1 - ml); }
				}
				for (int16_t col = 0; col < box.w; ++col) {
					bool lit = gfx.pixels[(box.y + row) * 120 + box.x + col] != 0;
					if (lit) { assert(col >= left && col <= right); ++count; }
					assert(lit == (col >= left && col <= right));
				}
			}
			assert(count > 0);
			if (style == DRL && height > 6) {
				assert(gfx.pixels[(box.y + height / 2) * 120 + box.x + box.w / 2] == cfg.fgColor);
				eye_scene_body(&gfx, cfg, box, 0.01f);
				assert(gfx.pixels[(box.y + height / 2) * 120 + box.x + box.w / 2] == cfg.fgColor);
			}
			EyeState gaze; gaze.saccadeX = 30; gaze.tiltY = -30;
			cfg.pupilColor = 0xF800; cfg.highlightColor = 0x07E0;
			eye_pupils(&gfx, cfg, gaze, box);
			for (int16_t row = 0; row < height; ++row) {
				int16_t left, right; eye_body_span(cfg, box, row, left, right);
				for (int16_t col = 0; col < box.w; ++col) {
					uint16_t ink = gfx.pixels[(box.y + row) * 120 + box.x + col];
					if (ink == cfg.pupilColor || ink == cfg.highlightColor) { assert(col >= left && col <= right); }
				}
			}
		}
	}
}

/** @brief Kitsune shares Normal geometry; DRL palette changes only its filled center. */
static void signature_palette_tests() {
	Arduino_GFX gfx(240, 280); eyes_init(&gfx);
	eyes_set_style(NORMAL); EyeConfig normal = eyes_config();
	eyes_set_style(KITSUNE);
	assert(eyes_config().eyeW == normal.eyeW && eyes_config().eyeH == normal.eyeH);
	assert(eyes_config().eyeGap == normal.eyeGap && eyes_config().radius == normal.radius);
	eyes_set_style(DRL);
	EyeBox box = {30, 50, 84, 50, 0, true};
	std::vector<bool> rim;
	for (uint16_t color : {uint16_t(0x07FF), uint16_t(0xFD6C), uint16_t(0xF9D6), uint16_t(0xFFFF)}) {
		EyeConfig cfg = eyes_config(); cfg.expression = NEUTRAL;
		cfg.fgColor = color; cfg.bgColor = 0; cfg.highlightColor = 0x07E0;
		gfx.fillScreen(0); eye_body(&gfx, cfg, box);
		assert(gfx.pixels[(box.y + box.h / 2) * 240 + box.x + box.w / 2] == color);
		if (rim.empty()) {
			for (uint16_t pixel : gfx.pixels) { rim.push_back(pixel == 0xFFFF); }
			assert(std::count(rim.begin(), rim.end(), true) > 0);
		}
		for (size_t i = 0; i < gfx.pixels.size(); ++i) {
			if (rim[i]) { assert(gfx.pixels[i] == 0xFFFF); }
			assert(gfx.pixels[i] == 0 || gfx.pixels[i] == 0xFFFF || gfx.pixels[i] == color);
		}
	}
}

/** @brief Both physical panels show distinct new styles and erase moving/blinking geometry cleanly. */
static void signature_style_tests() {
	signature_profile_tests();
	signature_palette_tests();
	for (EyeStyle style : {ANIME, KITSUNE, DRL}) { signature_span_tests(style); }
	for (auto panel : {std::pair<int, int>{240, 280}, {368, 448}}) {
		Arduino_GFX gfx(panel.first, panel.second); eyes_init(&gfx);
		std::vector<uint16_t> previous;
		for (EyeStyle style : {ANIME, KITSUNE, DRL}) {
			eyes_set_style(style); eyes_set_expression(NEUTRAL);
			eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0); eyes_draw();
			assert(gfx.pixels != previous); previous = gfx.pixels;
			auto &state = eyes_state(); state.saccadeX = 30; state.tiltY = -18;
			for (bool blink : {true, false}) {
				state.blinking = blink; eyes_draw(); auto dirty = gfx.pixels;
				state.dirtyInit = false; eyes_draw(); assert(gfx.pixels == dirty);
			}
			assert((std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xF800) > 0) == (style != DRL));
			assert((std::count(gfx.pixels.begin(), gfx.pixels.end(), 0x07E0) > 0) == (style == ANIME));
		}
	}
}

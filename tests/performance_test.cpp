/**
 * @file performance_test.cpp
 * @brief Original performance moods: palettes, reversible cycles, raster bounds and stable preferences.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_performances.h"
#include "../OTA-bot-Mochi/src/eyes/eye_performance_draw.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <cstring>

/** @brief All new moods round-trip through names, both languages and unchanged v5 bytes. */
static void preferences() {
	assert(HEADLIGHTS == 96 && DANCING == 97 && LOOK_DOWN == 116 && STARGAZING == 117 && ENRAGED == 122);
	for (uint8_t mood = DANCING; mood < EXPRESSION_COUNT; ++mood) {
		CompanionSettings p = settings_get();
		p.mood = mood;
		uint8_t bytes[SETTINGS_BYTES];
		settings_encode(p, bytes);
		CompanionSettings decoded;
		assert(bytes[0] == 9 && SETTINGS_BYTES == 16);
		assert(settings_decode(bytes, sizeof(bytes), decoded) && decoded.mood == mood);
		String command = String("mood ") + settings_mood_name(mood);
		auto parsed = settings_parse(command.c_str(), p);
		assert(parsed.action == UsbAction::Set && parsed.profile.mood == mood);
		assert(eyes_set_expression_by_name(settings_mood_name(mood)));
		for (uint8_t language = 0; language < 2; ++language) {
			p.language = language; assert(settings_update(p));
			assert(std::strcmp(tr_expression(mood), "?") != 0);
		}
	}
}

/** @brief Count four-connected painted objects, irrespective of their internal colors. @param gfx Canvas. @return Component count. */
static unsigned components(const Arduino_GFX &gfx) {
	std::vector<bool> seen(gfx.pixels.size(), false);
	unsigned count = 0;
	for (size_t start = 0; start < gfx.pixels.size(); ++start) {
		if (!gfx.pixels[start] || seen[start]) { continue; }
		++count;
		std::vector<size_t> pending{start};
		seen[start] = true;
		while (!pending.empty()) {
			size_t at = pending.back(); pending.pop_back();
			int x = int(at % gfx.width()), y = int(at / gfx.width());
			const int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
			for (uint8_t d = 0; d < 4; ++d) {
				int nx = x + dx[d], ny = y + dy[d];
				if (nx < 0 || ny < 0 || nx >= gfx.width() || ny >= gfx.height()) { continue; }
				size_t next = size_t(ny) * gfx.width() + nx;
				if (seen[next] || !gfx.pixels[next]) { continue; }
				seen[next] = true; pending.push_back(next);
			}
		}
	}
	return count;
}

/** @brief The two morphing eyes must become ordinary moving columns, not extra frozen bars. @param cfg Config. @param layout Eyes. */
static void equalizer_regression(const EyeConfig &cfg, const EyeLayout &layout) {
	Arduino_GFX actual(cfg.screenW, cfg.screenH), expected(cfg.screenW, cfg.screenH);
	for (uint32_t t : {1500u, 1850u, 3000u, 4400u}) {
		actual.fillScreen(0); expected.fillScreen(0);
		eye_performance_equalizer(&actual, cfg, layout, t, 1);
		for (uint8_t i = 0; i < 12; ++i) {
			EyeBox bar = eye_equalizer_bar(cfg, layout, i, t);
			expected.fillRoundRect(bar.x, bar.y, bar.w, bar.h, bar.radius, eye_equalizer_ink(i));
		}
		assert(actual.pixels == expected.pixels);
		assert(components(actual) == 12);
	}
	for (uint32_t t : {625u, 750u, 1000u, 1250u, 4750u, 5000u, 5375u}) {
		float a = eye_performance_amount(t);
		actual.fillScreen(0);
		eye_performance_equalizer(&actual, cfg, layout, t, a);
		for (const EyeBox &eye : {layout.left, layout.right}) {
			expected.fillScreen(0);
			uint8_t index = eye.left ? 2 : 9;
			EyeConfig body = cfg;
			body.fgColor = eye_mix(eye_equalizer_ink(index), cfg.fgColor, uint8_t(a * 100));
			eye_scene_body(&expected, body, eye_scene_box(eye, eye_equalizer_bar(cfg, layout, index, t), a), a);
			for (size_t pixel = 0; pixel < expected.pixels.size(); ++pixel) {
				if (expected.pixels[pixel]) { assert(actual.pixels[pixel] == expected.pixels[pixel]); }
			}
		}
	}
}

/** @brief Intake hubs face inward, housings are large and smoke fades in short bursts. @param cfg Config. @param layout Eyes. */
static void turbo_regressions(EyeConfig cfg, const EyeLayout &layout) {
	cfg.expression = ENERGETIC;
	Arduino_GFX gfx(cfg.screenW, cfg.screenH);
	for (const EyeBox &eye : {layout.left, layout.right}) {
		EyeBox target = eye_turbo_target(cfg, eye.left);
		assert(target.w >= cfg.screenW / 3 - 2);
		gfx.fillScreen(0);
		eye_performance_turbo(&gfx, cfg, eye, 3450, 1);
		auto first = gfx.pixels;
		assert(std::count(first.begin(), first.end(), eye_rgb(145, 165, 185)) > 0);
		int64_t totalX = 0, count = 0;
		for (size_t p = 0; p < first.size(); ++p) {
			if (first[p] == eye_rgb(225, 175, 105)) { totalX += p % cfg.screenW; ++count; }
		}
		assert(count > 0);
		int16_t cx = target.x + target.w / 2;
		assert(eye.left ? totalX / count > cx : totalX / count < cx);
		gfx.fillScreen(0);
		eye_performance_turbo(&gfx, cfg, eye, 3600, 1);
		assert(gfx.pixels != first);
		gfx.fillScreen(0);
		eye_turbo_smoke(&gfx, cfg, cx, target.y + target.h / 2, target.radius, eye.left, 3450, 1);
		assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0) < int(gfx.pixels.size()));
		auto smoke = gfx.pixels;
		gfx.fillScreen(0);
		eye_turbo_smoke(&gfx, cfg, cx, target.y + target.h / 2, target.radius, eye.left, 3650, 1);
		assert(gfx.pixels != smoke);
	}
}

/** @brief Rain first enters at the top and later reaches the lower canvas. @param cfg Config. */
static void rain_regression(EyeConfig cfg) {
	cfg.expression = RAIN;
	Arduino_GFX gfx(cfg.screenW, cfg.screenH);
	eye_performance_rain(&gfx, cfg, 610, 1);
	assert(std::count(gfx.pixels.begin(), gfx.pixels.begin() + cfg.screenW * 8, 0) < cfg.screenW * 8);
	assert(std::all_of(gfx.pixels.begin() + cfg.screenW * 8, gfx.pixels.end(), [](uint16_t c) { return c == 0; }));
	gfx.fillScreen(0);
	eye_performance_rain(&gfx, cfg, 1700, 1);
	assert(std::count(gfx.pixels.begin() + gfx.pixels.size() / 2, gfx.pixels.end(), 0) < int(gfx.pixels.size() / 2));
}

/** @brief Sushi is one joined object, and turbochargers have metal housings, outlet pipes and moving blades. @param cfg Config. @param layout Eyes. */
static void object_regressions(EyeConfig cfg, const EyeLayout &layout) {
	Arduino_GFX gfx(cfg.screenW, cfg.screenH);
	cfg.expression = SUSHI;
	eye_performance_sushi(&gfx, cfg, layout, 3000, 1);
	assert(components(gfx) == 1);
	EyeBox rice = eye_sushi_target(cfg);
	assert(gfx.pixels[size_t(cfg.screenH / 2) * cfg.screenW + cfg.screenW / 2] != 0);
	assert(gfx.pixels[size_t(cfg.screenH / 2) * cfg.screenW + rice.x - 1] == 0);
	turbo_regressions(cfg, layout);
	rain_regression(cfg);
}

/** @brief Validate the requested subjects and starry-sky gaze on each style. @param width Panel width. @param height Panel height. */
static void scene_regressions(int16_t width, int16_t height) {
	Arduino_GFX gfx(width, height);
	eyes_init(&gfx);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		EyeConfig cfg = eyes_config();
		cfg.expression = DANCING;
		EyeLayout layout = eye_layout(cfg, EyeState{});
		equalizer_regression(cfg, layout);
		object_regressions(cfg, layout);
		cfg.expression = STARGAZING;
		EyeLayout sky = layout;
		eye_performance_layout(cfg, sky, 3000);
		for (const EyeBox &eye : {sky.left, sky.right}) {
			assert(eye.y > height * 3 / 4 && eye.y + eye.h <= height - height / 20);
			assert(eye.w <= layout.left.w / 2 && eye.h <= layout.left.h / 2);
		}
		assert(sky.left.x + sky.left.w < sky.right.x);
		assert(sky.right.x + sky.right.w - sky.left.x < width / 3);
		EyeLayout settled = layout;
		eye_performance_layout(cfg, settled, 1500);
		assert(sky.left.y + sky.left.h / 2 < settled.left.y + settled.left.h / 2);
		gfx.fillScreen(0);
		eye_performance_background(&gfx, cfg, sky, 0);
		assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0) == int(gfx.pixels.size()));
		eye_performance_background(&gfx, cfg, sky, 3000);
		assert(std::count(gfx.pixels.begin(), gfx.pixels.begin() + width * height / 3, 0) < width * height / 3);
		auto stars = gfx.pixels;
		gfx.fillScreen(0); eye_performance_background(&gfx, cfg, sky, 3500);
		assert(gfx.pixels != stars);
	}
}

/** @brief Pupilled styles look upward within the small bottom eyes; solid styles remain pupil-free. */
static void upward_pupils() {
	Arduino_GFX gfx(368, 448);
	eyes_init(&gfx);
	for (EyeStyle style : {NORMAL, BLOUB}) {
		eyes_set_style(style); eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		eyes_life_reset(); eyes_set_expression(STARGAZING);
		eyes_state().lastMs = 0; eyes_draw();
		eyes_state().lastMs = 3000; eyes_draw();
		EyeLayout layout = eye_layout(eyes_config(), EyeState{});
		eye_performance_layout(eyes_config(), layout, 3000);
		int64_t totalY = 0, count = 0;
		for (int y = 0; y < gfx.height(); ++y) {
			for (int x = 0; x < gfx.width(); ++x) {
				if (gfx.pixels[size_t(y) * gfx.width() + x] != 0xF800) { continue; }
				totalY += y; ++count;
			}
		}
		assert(count > 0 && totalY / count < layout.left.y + layout.left.h / 2);
	}
	assert(eye_stargazing_look(1500) == 0 && eye_stargazing_look(2100) == 1);
	assert(eye_stargazing_look(3900) == 1 && eye_stargazing_look(4500) == 0);
	assert(eye_stargazing_look(1800) == eye_stargazing_look(4200));
}

/** @brief Real rendered entry time, including rollover, protects every six-second cycle. */
static void automatic_guard() {
	Arduino_GFX gfx(240, 280);
	eyes_init(&gfx);
	for (uint8_t mood = DANCING; mood < EXPRESSION_COUNT; ++mood) {
		CompanionSettings p = settings_get();
		p.mood = mood; p.random_faces = p.imu_faces = p.shake = false;
		assert(settings_update(p));
		eyes_life_reset();
		test_now = UINT32_MAX - 3000;
		eyes_state().lastMs = test_now;
		eyes_draw();
		assert(!eyes_faces_automatic_ready(test_now));
		uint32_t length = FacePolicy::cycle_duration(Expression(mood));
		assert(!eyes_faces_automatic_ready(test_now + length - 1));
		assert(eyes_faces_automatic_ready(test_now + length));
		eyes_set_expression(static_cast<Expression>(mood));
		assert(!eyes_faces_automatic_ready(test_now + 6000));
	}
}

/** @brief Every frame equals a clean redraw, and each performance returns to its initial eyes. @param width Panel width. @param height Panel height. */
static void raster(int16_t width, int16_t height) {
	Arduino_GFX gfx(width, height);
	eyes_init(&gfx);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		for (uint8_t value = DANCING; value < EXPRESSION_COUNT; ++value) {
			Expression mood = static_cast<Expression>(value);
			eyes_life_reset(); eyes_set_expression(mood);
			EyeState &st = eyes_state();
			st.lastMs = 10000;
			eyes_draw();
			uint32_t length = FacePolicy::cycle_duration(mood);
			st.lastMs = 10000 + length; eyes_draw();
			auto first = gfx.pixels;
			bool changed = false, colored = false;
			for (uint32_t t = 0; t <= length; t += 125) {
				st.lastMs = 10000 + t;
				eyes_draw();
				auto frame = gfx.pixels;
				changed |= frame != first;
				colored |= std::any_of(frame.begin(), frame.end(), [](uint16_t c) {
					return c != 0 && c != 0xFFFF && c != 0xF800 && c != 0x07E0;
				});
				st.dirtyInit = false; eyes_draw();
				assert(gfx.pixels == frame);
				assert(st.prevX >= 0 && st.prevY >= 0);
				assert(st.prevX + st.prevW <= width && st.prevY + st.prevH <= height);
				if (t >= length - 500) { assert(frame == first); }
				if (style == MOCHI || style == TRAPEZ) {
					assert(std::count(frame.begin(), frame.end(), 0xF800) == 0);
					assert(std::count(frame.begin(), frame.end(), 0x07E0) == 0);
				}
			}
			assert(changed && colored);
			assert(eyes_config().fgColor == 0xFFFF);
			eyes_set_expression(NEUTRAL);
			eyes_draw();
			auto returned = gfx.pixels;
			st.dirtyInit = false; eyes_draw();
			assert(gfx.pixels == returned);
		}
	}
}

/** @brief Semantic tint is frame-local for every selected base color. */
static void colors() {
	for (uint8_t value = DANCING; value < EXPRESSION_COUNT; ++value) {
		EyeConfig cfg{}; cfg.expression = static_cast<Expression>(value);
		for (uint16_t base : {uint16_t(0xFFFF), uint16_t(0x07FF), uint16_t(0xFC80), uint16_t(0x001F)}) {
			cfg.fgColor = base;
			for (bool left : {false, true}) {
				assert(eye_performance_ink(cfg, left, 0) == base);
				assert(eye_performance_ink(cfg, left, 5400) == base);
				assert(eye_performance_ink(cfg, left, 6000) == base);
			}
		}
	}
}

/** @brief Banners use our name, remain bounded, and disappear at both endpoints. */
static void banner_regressions() {
	for (auto panel : {std::pair<int16_t, int16_t>{240, 280}, {368, 448}, {128, 160}, {160, 128}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		EyeConfig cfg{}; cfg.screenW = panel.first; cfg.screenH = panel.second;
		cfg.fgColor = 0xFFFF;
		for (Expression mood : {MISTAKE, ENCOURAGEMENT}) {
			cfg.expression = mood;
			for (uint32_t elapsed : {0u, 1500u, 3000u, 4500u, 5400u, 6000u}) {
				gfx.text.clear();
				eye_performance_background(&gfx, cfg, EyeLayout{}, elapsed);
				if (elapsed == 0 || elapsed >= 5400) { assert(gfx.text.empty()); continue; }
				assert(!gfx.text.empty());
				for (const auto &line : gfx.text) {
					assert(line == (mood == MISTAKE ? "OTA BOT!" : "GO OTA BOT!"));
				}
			}
		}
	}
}

/** @brief Appended GIF themes survive rollover and non-black dirty redraws on every panel/style. */
static void appended_cycles() {
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	assert(TRAFFIC == 123 && YOKAI == 133 && TIC_TAC_TOE == 134 && EXPRESSION_COUNT == 135);
	for (const auto &panel : panels) {
		Arduino_GFX gfx(panel[0], panel[1]);
		eyes_init(&gfx);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_set_style(EyeStyle(style));
			eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0);
			for (uint8_t id = TRAFFIC; id < EXPRESSION_COUNT; ++id) {
				eyes_state() = EyeState{};
				eyes_set_expression(Expression(id));
				uint32_t entry = UINT32_MAX - 1000;
				eyes_state().lastMs = entry; eyes_draw();
				uint32_t length = FacePolicy::cycle_duration(Expression(id));
				eyes_state().lastMs = entry + length; eyes_draw();
				auto first = gfx.pixels;
				for (uint32_t t = 125; t <= length; t += 125) {
					eyes_state().lastMs = entry + t; eyes_draw();
					auto frame = gfx.pixels;
					eyes_state().dirtyInit = false; eyes_draw();
					assert(gfx.pixels == frame);
					if (t >= length - 500) { assert(frame == first); }
				}
				assert(eyes_config().fgColor == 0xFFFF && eyes_config().bgColor == 0x0841);
			}
		}
	}
}

/** @brief Validate new moods without running device firmware or copying reference assets. @return Success. */
int main() {
	assert(settings_init());
	CompanionSettings saved = settings_get();
	preferences(); automatic_guard(); colors(); banner_regressions();
	raster(240, 280); raster(368, 448); raster(128, 160); raster(160, 128);
	scene_regressions(240, 280); scene_regressions(368, 448);
	scene_regressions(128, 160); scene_regressions(160, 128);
	upward_pupils();
	appended_cycles();
	assert(settings_update(saved));
	std::cout << "38 colored performances, original scene regressions and appended scene cycles passed\n";
}

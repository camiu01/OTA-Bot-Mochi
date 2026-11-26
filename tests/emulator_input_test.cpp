/**
 * @file emulator_input_test.cpp
 * @brief SDL logical coordinates, fast clicks, releases and cancelled contacts.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../tools/emulator.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/ui/notification_glow.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/alert_layout.h"
#include <cassert>
#include <cstdio>
#include <vector>

void boot_firmware();
void handle_auto_page(uint32_t now);
void render_frame(SDL_Renderer *, SDL_Texture *, SDL_Window *, uint32_t notifications);
const std::vector<uint16_t> &visible_pixels();

/** @brief Deliver one logical mouse transition. @param type SDL type. @param x Logical X. @param y Logical Y. */
static void mouse(uint32_t type, int x, int y) {
	SDL_Event event{};
	event.type = type;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.x = x; event.button.y = y;
	bool running = true;
	handle_event(event, nullptr, running);
}

/** @brief Clear gesture state between independent cases. */
static void reset_contact() {
	finger = false; pending_step = 0; flick_left = 0;
	actions = TouchActions{}; swipe = SwipeTracker{};
	switch_screen(UiPage::Eyes, test_now);
}

/** @brief Check terminal line editing, CRLF, bounded input and real command execution. */
static void terminal_commands() {
	Serial.clear();
	for (char c : std::string("mood gloX\bw\r\n")) { terminal_character(c); }
	assert(Serial.input == "mood glow\n");
	settings_usb_poll();
	assert(settings_get().mood == uint8_t(Expression::GLOW));
	for (char c : std::string("help\nsound\n")) { terminal_character(c); }
	assert(Serial.input.substr(Serial.position) == "help\nsound\n");
	settings_usb_poll();
	assert(Serial.available() == 0);
	for (int i = 0; i < 200; ++i) { terminal_character('x'); }
	terminal_character('\n');
	assert(Serial.available() == 0);
	for (char c : std::string("mood headlights\n")) { terminal_character(c); }
	settings_usb_poll();
	assert(settings_get().mood == uint8_t(Expression::HEADLIGHTS));
}

/** @brief Dummy sessions never request acceleration; full and partial startup cleanly release SDL. */
static void display_lifecycle() {
	assert(emulator_renderer_flags("dummy") == SDL_RENDERER_SOFTWARE);
	assert(emulator_renderer_flags("windows") == SDL_RENDERER_ACCELERATED);
	assert(emulator_renderer_flags("x11") == SDL_RENDERER_ACCELERATED);
	assert(emulator_renderer_flags(nullptr) == SDL_RENDERER_ACCELERATED);
	assert(SDL_setenv("SDL_VIDEODRIVER", "dummy", 1) == 0);
	for (unsigned stage = 0; stage < 4; ++stage) {
		assert(SDL_Init(SDL_INIT_VIDEO) == 0);
		assert(!emulator_create_renderer(nullptr));
		SDL_Window *window = nullptr;
		SDL_Renderer *renderer = nullptr;
		SDL_Texture *texture = nullptr;
		if (stage >= 1) {
			window = SDL_CreateWindow("lifecycle", 0, 0, LCD_W, LCD_H, SDL_WINDOW_HIDDEN);
			assert(window);
		}
		if (stage >= 2) {
			renderer = emulator_create_renderer(window); assert(renderer);
			SDL_RendererInfo info{}; assert(SDL_GetRendererInfo(renderer, &info) == 0);
			assert(info.flags & SDL_RENDERER_SOFTWARE);
			assert(!(info.flags & SDL_RENDERER_ACCELERATED));
		}
		if (stage >= 3) {
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_W, LCD_H);
			assert(texture); SDL_StartTextInput();
		}
		emulator_close_display(window, renderer, texture);
		assert(SDL_WasInit(SDL_INIT_VIDEO) == 0);
	}
	std::puts("Dummy renderer selection and partial/full SDL lifecycle passed");
}

/** @brief Ensure the bottom of the card and the action gap are free of clipped text. */
static void alert_footer_bounds() {
	for (int y = ui_s(214); y < alert_layout::button_top(); ++y) {
		for (int x = ui_s(24); x < LCD_W - ui_s(24); ++x) {
			const auto pixel = gfx.pixels[y * LCD_W + x];
			assert(pixel == UI_BG || (y < ui_s(218) && (pixel == UI_CARD || pixel == UI_LINE)));
		}
	}
}

/** @brief Verify that N reaches the real alert path and that the ring fades away. */
static void notification_key() {
	assert(SDL_Init(SDL_INIT_VIDEO) == 0);
	auto *window = SDL_CreateWindow("notification", 0, 0, LCD_W, LCD_H, SDL_WINDOW_HIDDEN);
	assert(window);
	auto *renderer = emulator_create_renderer(window);
	assert(renderer);
	auto *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_W, LCD_H);
	assert(texture);
	constexpr uint32_t start = 50000;
	const auto paint = [&](uint32_t now) {
		test_now = now;
		render_frame(renderer, texture, window, ui_page_events().notifications);
		return visible_pixels() != gfx.pixels;
	};
	assert(!paint(start));
	const uint32_t before = ui_page_events().notifications;
	SDL_Event key{};
	key.type = SDL_KEYDOWN; key.key.keysym.sym = SDLK_n;
	bool running = true;
	handle_event(key, renderer, running);
	assert(ui_page_events().notifications == before + 1);
	handle_auto_page(test_now);
	assert(page == UiPage::Notification);
	assert(!paint(start));
	assert(paint(start + notification_glow::FadeInMs) == notification_glow::Supported);
	assert(paint(start + 10000) == notification_glow::Supported);
	assert(paint(start + notification_glow::DurationMs - 250) == notification_glow::Supported);
	assert(!paint(start + notification_glow::DurationMs) && page == UiPage::Notification);
	alert_footer_bounds();
	assert(gfx.pixels[ui_s(114) * LCD_W + ui_s(28)] == UI_LINE);
	Notification long_note;
	long_note.app = "A long notification application name";
	long_note.title = "A long title takes two lines and may need to be shortened";
	long_note.message = "This longer message reaches the bottom of the alert card, then continues on the next page. "
		"The counters must remain inside the card and keep the action buttons below it clear.";
	long_note.time = "23:59";
	ui_notification(long_note);
	const uint32_t second = start + notification_glow::DurationMs + 30;
	paint(second);
	alert_footer_bounds();
	assert(gfx.pixels[ui_s(134) * LCD_W + ui_s(28)] == UI_LINE);
	assert(paint(second + 500) == notification_glow::Supported);
	const auto retained = ui_snapshot().notification_count;
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W / 2, LCD_H / 2);
	test_now += 40;
	mouse(SDL_MOUSEBUTTONUP, LCD_W / 2, LCD_H / 2);
	handle_touch(test_now);
	assert(!paint(test_now) && ui_snapshot().notification_count == retained);
	assert(!paint(test_now + 100));
	handle_event(key, renderer, running);
	const auto third = test_now;
	assert(!paint(third));
	assert(paint(third + 500) == notification_glow::Supported);
	const int lower_button_y = alert_layout::button_bottom() - 3;
	assert(lower_button_y >= ui_s(256));
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W / 2, lower_button_y);
	test_now += 40;
	mouse(SDL_MOUSEBUTTONUP, LCD_W / 2, lower_button_y);
	handle_touch(test_now);
	assert(!paint(test_now) && ui_snapshot().notification_count == retained);
	while (ui_snapshot().has_notification) { ui_notification_dismiss(); }
	assert(!paint(test_now + 100) && gfx.pixels[lower_button_y * LCD_W + LCD_W / 2] == UI_BG);
	emulator_close_display(window, renderer, texture);
	std::puts("Notification key, ten-second ring, tap dismissal and alert actions passed");
}

/** @brief Run deterministic event-level input regressions. @return Success. */
int main() {
	display_lifecycle();
	boot_firmware();
	test_now = 10000;
	reset_contact();
	SDL_Event wake{}; wake.type = SDL_KEYDOWN; wake.key.keysym.sym = SDLK_u;
	auto wake_profile = settings_get(); wake_profile.imu_faces = false;
	assert(settings_update(wake_profile));
	bool ignored = true; handle_event(wake, nullptr, ignored); assert(!eyes_state().wakeUntilMs);
	wake_profile.imu_faces = true; assert(settings_update(wake_profile));
	bool waking = true; handle_event(wake, nullptr, waking);
	assert(eyes_state().wakeUntilMs == test_now + 1400);
	// SDL_RenderSetLogicalSize already scales mouse events, even at 3x or after resizing.
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W * 3 / 4, LCD_H / 2);
	assert(finger_x == LCD_W * 3 / 4 && finger_y == LCD_H / 2);
	assert(actions.active);
	assert(eyes_state().tracking && eyes_state().saccadeX > 0);
	mouse(SDL_MOUSEBUTTONUP, LCD_W * 3 / 4, LCD_H / 2);
	assert(!finger && !actions.active && actions.tap_armed);
	assert(!eyes_state().tracking);
	// A second complete click in the same input batch must not disappear.
	test_now += 100;
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W * 3 / 4, LCD_H / 2);
	mouse(SDL_MOUSEBUTTONUP, LCD_W * 3 / 4, LCD_H / 2);
	assert(!actions.tap_armed);
	reset_contact();
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W / 2, LCD_H / 2);
	test_now += 1200; sample_touch(test_now); handle_touch(test_now);
	assert(page == UiPage::Settings);
	reset_contact();
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W * 3 / 4, LCD_H / 2);
	test_now += 100;
	mouse(SDL_MOUSEBUTTONUP, LCD_W / 4, LCD_H / 2);
	assert(page != UiPage::Eyes && !actions.tap_armed);
	reset_contact();
	mouse(SDL_MOUSEBUTTONDOWN, -10, LCD_H / 2);
	assert(!finger && !actions.active);
	mouse(SDL_MOUSEBUTTONDOWN, LCD_W / 2, LCD_H / 2);
	SDL_Event lost{};
	lost.type = SDL_WINDOWEVENT; lost.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
	bool running = true;
	handle_event(lost, nullptr, running);
	assert(!finger && !actions.active && !actions.tap_armed);
	handle_touch(test_now);
	assert(!eyes_state().tracking);
	terminal_commands();
	notification_key();
	std::puts("Terminal command editing, CRLF, overflow recovery and execution passed");
	std::puts("Emulator logical coordinates, fast/double click, hold, drag and focus loss passed");
}

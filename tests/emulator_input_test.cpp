/**
 * @file emulator_input_test.cpp
 * @brief SDL logical coordinates, fast clicks, releases and cancelled contacts.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../tools/emulator.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <cstdio>

void boot_firmware();

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
	std::puts("Terminal command editing, CRLF, overflow recovery and execution passed");
	std::puts("Emulator logical coordinates, fast/double click, hold, drag and focus loss passed");
}

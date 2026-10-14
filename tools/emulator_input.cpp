/**
 * @file emulator_input.cpp
 * @brief Window input translated into the firmware's production gesture policies.
 *
 * Mouse contact samples go through the same SwipeTracker and TouchActions the
 * device uses, so taps, holds, double taps, petting, scrolls, and page swipes
 * behave exactly as on hardware. The USB console forwards typed lines to the
 * real settings_usb_poll() parser.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "emulator.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include <cstdio>
#include <string>

extern bool console_prompt;

namespace {
bool console = false;
std::string console_line;

/** @brief Advance the synthetic flick one contact sample at a time. */
void advance_flick() {
	--flick_left;
	if (flick_left >= 3) { finger = true; finger_x = int16_t(LCD_W / 2); finger_y = int16_t(LCD_H / 2); }
	else if (flick_left >= 1) {
		finger = true;
		finger_y = int16_t(LCD_H / 2 + flick_dir * 32 * (4 - flick_left));
	} else { finger = false; flick_left = 0; flick_dir = 0; }
}

/** @brief Start a synthetic vertical flick, used by the mouse wheel. @param dir -1 moves up, +1 down. */
void begin_flick(int8_t dir) {
	if (finger || flick_left) { return; }
	flick_dir = dir;
	flick_left = 4;
}

/** @brief Translate a window coordinate into logical LCD pixels. @param renderer Renderer. @param wx Window X. @param wy Window Y. @param x Output X. @param y Output Y. */
void window_to_lcd(SDL_Renderer *renderer, int wx, int wy, int16_t &x, int16_t &y) {
	float lx = 0, ly = 0;
	SDL_RenderWindowToLogical(renderer, float(wx), float(wy), &lx, &ly);
	x = int16_t(lx < 0 ? 0 : (lx > LCD_W - 1 ? LCD_W - 1 : lx));
	y = int16_t(ly < 0 ? 0 : (ly > LCD_H - 1 ? LCD_H - 1 : ly));
}

/** @brief Open the USB console with a fresh prompt. */
void open_console() {
	console = true;
	std::fputs("> ", stdout);
	std::fflush(stdout);
}

/** @brief Handle console editing keys. @param key Key code. @param running Loop flag. @return True when the key was consumed. */
bool console_key(SDL_Keycode key) {
	if (!console) { return false; }
	if (key == SDLK_ESCAPE) {
		console = false;
		console_line.clear();
		std::fputc('\n', stdout);
	} else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		if (console_line.empty()) { console = false; std::fputc('\n', stdout); }
		else {
			Serial.input += console_line + "\n";
			console_line.clear();
			console_prompt = true;
			std::fputc('\n', stdout);
		}
	} else if (key == SDLK_BACKSPACE && !console_line.empty()) {
		console_line.pop_back();
		std::fputs("\b \b", stdout);
	}
	std::fflush(stdout);
	return true;
}

/** @brief Handle single key presses outside the console. @param key Key code. @param running Loop flag. */
void handle_key(SDL_Keycode key, bool &running) {
	if (console_key(key)) { return; }
	if (key == SDLK_ESCAPE || key == SDLK_q) { running = false; return; }
	if (key == SDLK_RETURN || key == SDLK_KP_ENTER) { open_console(); return; }
	if (key == SDLK_p) { write_ppm(shot_path); return; }
	if (key == SDLK_n) { inject_notification(); return; }
	if (key == SDLK_r) { toggle_route(); return; }
	static const UiPage jump[] = {UiPage::Eyes, UiPage::Clock, UiPage::Navigation,
		UiPage::Notification, UiPage::Weather, UiPage::Settings, UiPage::Desk};
	if (key >= SDLK_0 && key <= SDLK_6) { switch_screen(jump[key - SDLK_0], test_now); }
}
}

void sample_touch(uint32_t now) {
	if (flick_left) { advance_flick(); }
	actions.update(finger, finger_x, finger_y, now);
	if (actions.vertical) {
		swipe.update(false, 0, 0, now);
		pending_step = 0;
		return;
	}
	pending_step = swipe.update(finger, finger_x, finger_y, now);
}

namespace {
/** @brief One contact's pending gestures, consumed like the touch accessors. */
struct Gesture {
	bool hold = false, tap = false, dbl = false, pet = false;
	int8_t step = 0, scroll = 0;
	int16_t x = 0, y = 0, pet_x = 0, pet_y = 0;
};

/** @brief Consume the flags the gesture policies staged since the last call. @return Pending gestures. */
Gesture take_gesture() {
	Gesture g;
	g.step = pending_step;
	pending_step = 0;
	g.hold = actions.hold;
	actions.hold = false;
	g.tap = actions.tap;
	g.x = actions.x;
	g.y = actions.y;
	actions.tap = false;
	g.dbl = actions.take_double();
	g.pet = actions.take_pet(g.pet_x, g.pet_y);
	g.scroll = actions.scroll;
	actions.scroll = 0;
	return g;
}
}

void handle_touch(uint32_t now) {
	const Gesture g = take_gesture();
	if (g.tap && g.y < 50 && !screens_calling() && page != UiPage::Eyes && page != UiPage::Settings) {
		toggle_quick_sound(now);
		return;
	}
	if (g.scroll && page != UiPage::Eyes && !screens_calling()) {
		if (page == UiPage::Settings) {
			if (settings_menu_scroll(g.scroll)) { screens_invalidate(); }
			return;
		}
		UiPage target = screens_scroll(page, g.scroll);
		if (target != page) { switch_screen(target, now); }
		return;
	}
	if (g.hold && page == UiPage::Eyes && !screens_calling()) {
		switch_screen(UiPage::Settings, now);
		return;
	}
	if (g.hold && page == UiPage::Navigation && !screens_calling()) {
		view_route_temporary(-1);
		screens_invalidate();
		return;
	}
	if (g.pet && page == UiPage::Eyes && !screens_calling()) { eyes_faces_pet(now, g.pet_x, g.pet_y); }
	if (g.tap && page == UiPage::Settings && !screens_calling()) {
		MenuAction action = settings_menu_tap(g.x, g.y);
		if (action == MenuAction::Exit) { switch_screen(UiPage::Eyes, now); }
		else if (action == MenuAction::Changed) { screens_invalidate(); }
	} else if (g.tap && !screens_calling()) {
		if (page == UiPage::Eyes) {
			if (g.dbl) { eyes_faces_double(now, g.x, g.y); }
			else { eyes_faces_tap(now, g.x, g.y); }
		} else {
			UiPage target = screens_tap(page, g.x, g.y, now);
			if (target != page) { switch_screen(target, now); }
		}
	}
	if (!g.step) { return; }
	switch_screen(screens_swipe(page, g.step), now);
}

void handle_event(const SDL_Event &event, SDL_Renderer *renderer, bool &running) {
	if (event.type == SDL_QUIT) { running = false; return; }
	if (event.type == SDL_TEXTINPUT && console) {
		console_line += event.text.text;
		std::fputs(event.text.text, stdout);
		std::fflush(stdout);
		return;
	}
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		finger = true;
		window_to_lcd(renderer, event.button.x, event.button.y, finger_x, finger_y);
		return;
	}
	if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) { finger = false; return; }
	if (event.type == SDL_MOUSEMOTION && finger) {
		window_to_lcd(renderer, event.motion.x, event.motion.y, finger_x, finger_y);
		return;
	}
	if (event.type == SDL_MOUSEWHEEL && event.wheel.y) {
		begin_flick(event.wheel.y > 0 ? -1 : 1);
		return;
	}
	if (event.type == SDL_KEYDOWN) { handle_key(event.key.keysym.sym, running); }
}

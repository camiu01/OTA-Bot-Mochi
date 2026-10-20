/**
 * @file lcd_emulator.cpp
 * @brief Host SDL2 window that drives the real UI code against the native raster.
 *
 * The emulator reuses the production gesture policies (SwipeTracker and
 * TouchActions), page routing, settings, sound/backlight services, USB command
 * parser, and the real eye renderer, so the window shows firmware logic
 * without a board. Input handling lives in emulator_input.cpp.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "emulator.h"
#include "../OTA-bot-Mochi/src/clock/clock_ui.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/power/backlight.h"
#include "../OTA-bot-Mochi/src/power/battery.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/src/ui/ui_state.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

Arduino_GFX gfx;
ChronosESP32 watch;
UiPage page = UiPage::Eyes;
SwipeTracker swipe;
TouchActions actions;
int8_t pending_step = 0;
bool finger = false;
int16_t finger_x = LCD_W / 2, finger_y = LCD_H / 2;
uint32_t last_sample = 0, last_frame = 0;
int8_t flick_dir = 0;
uint8_t flick_left = 0;
std::string shot_path = "mochi-lcd.ppm";
bool console_prompt = false;

namespace {
AutoPagePolicy auto_policy;
uint32_t limit_frames = 0, rendered = 0;
UiPage start_page = UiPage::Eyes;
std::string boot_command;
}

const char *page_name(UiPage value) {
	static const char *const names[] = {"Eyes", "Clock", "Route", "Alerts", "Weather",
		"Settings", "Desk", "Big clock", "Overview", "Timer", "Device", "Pong", "Analog clock"};
	return uint8_t(value) < sizeof(names) / sizeof(names[0]) ? names[uint8_t(value)] : "?";
}

bool page_from_name(const std::string &name, UiPage &out) {
	static const char *const keys[] = {"eyes", "clock", "route", "alerts", "weather",
		"settings", "desk", "bigclock", "overview", "timer", "device", "pong", "analogclock"};
	for (uint8_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
		if (name == keys[i]) { out = UiPage(i); return true; }
	}
	return false;
}

void switch_screen(UiPage next, uint32_t now) {
	page = next;
	gfx.fillScreen(UI_BG);
	screens_invalidate();
	eyes_state().dirtyInit = false;
	last_frame = now - FRAME_MS;
}

void toggle_quick_sound(uint32_t now) {
	CompanionSettings value = settings_get();
	value.sound = !value.sound;
	if (!settings_update(value)) { return; }
	if (value.sound) { sound_request(SoundCue::Tap, now); }
}

void inject_notification() {
	static const char *const senders[] = {"Messages", "Calendar", "Delivery"};
	static uint8_t count = 0;
	Notification note;
	note.app = senders[count % 3];
	note.title = "Mochi";
	note.message = "The emulator received a synthetic alert";
	++count;
	ui_notification(note);
}

void toggle_route() {
	bool on = !(watch.navigation.active && watch.navigation.isNavigation);
	watch.navigation.active = on;
	watch.navigation.isNavigation = on;
	ui_config(watch, CF_NAV_DATA, 1, 0);
}

/** @brief Wake the backlight on received data and update its touch policy. @param now Millisecond timestamp. */
void update_backlight(uint32_t now) {
	static uint32_t activity_revision = 0;
	uint32_t revision = ui_revision();
	if (revision != activity_revision) {
		activity_revision = revision;
		if (!settings_get().dashboard || page == UiPage::Notification ||
			page == UiPage::Navigation) { backlight_activity(now); }
	}
	backlight_update(now, finger, screens_keep_awake());
}

/** @brief Populate the fake Chronos source so weather, route, and clock pages have content. */
void seed_demo_data() {
	ui_connection(true);
	clock_time_synced();
	ui_config(watch, CF_TIME, 1, 0);
	static const int high[] = {21, 19, 17, 20, 22, 18, 16};
	for (uint8_t i = 0; i < 7; ++i) {
		watch.weather[i].icon = i;
		watch.weather[i].day = i;
		watch.weather[i].temp = high[i] - 5;
		watch.weather[i].high = high[i];
		watch.weather[i].low = high[i] - 9;
	}
	watch.weather_count = 7;
	ui_config(watch, CF_WEATHER, 1, 1);
	ui_config(watch, CF_WEATHER, 2, 0);
	Navigation &nav = watch.navigation;
	nav.title = "Via Roma";
	nav.directions = "Turn right at the next street";
	nav.distance = "450 m";
	nav.eta = "14:32";
	nav.duration = "6 min";
	nav.speed = "30 km/h";
	ui_config(watch, CF_NAV_DATA, 0, 0);
}

/** @brief Follow alert/route events without switching pages while a finger is down. @param now Millisecond timestamp. */
void handle_auto_page(uint32_t now) {
	UiPage target = auto_policy.update(page, ui_page_events(), finger);
	if (target != page) { backlight_activity(now); switch_screen(target, now); }
}

/** @brief Convert the RGB565 canvas to RGBA and present it scaled. @param renderer Renderer. @param texture Frame texture. */
void present(SDL_Renderer *renderer, SDL_Texture *texture) {
	static std::vector<uint32_t> frame(size_t(LCD_W) * LCD_H);
	for (size_t i = 0; i < frame.size(); ++i) {
		uint16_t p = gfx.pixels[i];
		uint32_t r = (p >> 11) & 0x1F, g = (p >> 5) & 0x3F, b = p & 0x1F;
		frame[i] = 0xFF000000u | ((((r << 3) | (r >> 2)) & 0xFF) << 16)
			| ((((g << 2) | (g >> 4)) & 0xFF) << 8) | (((b << 3) | (b >> 2)) & 0xFF);
	}
	SDL_UpdateTexture(texture, nullptr, frame.data(), LCD_W * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

/** @brief Draw one scheduled frame, mirroring the firmware loop's render stage. @param renderer Renderer. @param texture Frame texture. @param window Window for the title. */
void render_frame(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Window *window) {
	bool changed = screens_draw(&gfx, watch, page, test_now);
	if (page == UiPage::Eyes && !screens_calling()) {
		if (changed) { eyes_state().dirtyInit = false; }
		eyes_update(test_now);
		changed = true;
	}
	if (changed) { present(renderer, texture); }
	SDL_SetWindowTitle(window, (std::string("Mochi LCD — ") + page_name(page)).c_str());
}

bool write_ppm(const std::string &path) {
	std::FILE *file = std::fopen(path.c_str(), "wb");
	if (!file) { return false; }
	std::fprintf(file, "P6\n%d %d\n255\n", LCD_W, LCD_H);
	std::vector<uint8_t> row(size_t(LCD_W) * 3);
	for (int y = 0; y < LCD_H; ++y) {
		for (int x = 0; x < LCD_W; ++x) {
			uint16_t p = gfx.pixels[size_t(y) * LCD_W + x];
			uint32_t r = (p >> 11) & 0x1F, g = (p >> 5) & 0x3F, b = p & 0x1F;
			row[size_t(x) * 3] = uint8_t((r << 3) | (r >> 2));
			row[size_t(x) * 3 + 1] = uint8_t((g << 2) | (g >> 4));
			row[size_t(x) * 3 + 2] = uint8_t((b << 3) | (b >> 2));
		}
		std::fwrite(row.data(), 1, row.size(), file);
	}
	std::fclose(file);
	return true;
}

/** @brief Initialize services in the firmware's setup() order. */
void boot_firmware() {
	test_now = 0;
	settings_init();
	backlight_init(0);
	ui_state_init();
	eyes_init(&gfx);
	settings_apply();
	view_reset();
	seed_demo_data();
	eyes_faces_greet(0);
	sound_init(0);
	battery_sample(0);
}

/** @brief Forward USB command replies and console prompts to the terminal. */
void drain_serial() {
	if (!Serial.available()) { Serial.input.clear(); Serial.position = 0; }
	bool prompt = console_prompt;
	console_prompt = false;
	bool wrote = false;
	if (!Serial.output.empty()) {
		std::fputs(Serial.output.c_str(), stdout);
		Serial.output.clear();
		wrote = true;
	}
	if (prompt) { std::fputs("> ", stdout); wrote = true; }
	if (wrote) { std::fflush(stdout); }
}

/** @brief Run input, services, and scheduled rendering until the user quits. @return Process status. */
int event_loop(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Window *window) {
	bool running = true;
	uint32_t start = SDL_GetTicks();
	while (running) {
		test_now = SDL_GetTicks() - start;
		SDL_Event event;
		while (SDL_PollEvent(&event)) { handle_event(event, renderer, running); }
		test_now = SDL_GetTicks() - start;
		poll_terminal();
		settings_usb_poll();
		drain_serial();
		if (uint32_t(test_now - last_sample) >= SAMPLE_MS) {
			last_sample = test_now;
			sample_touch(test_now);
		}
		handle_touch(test_now);
		handle_auto_page(test_now);
		battery_sample(test_now);
		view_timer().tick(test_now);
		sound_poll(test_now, ui_page_events(), page == UiPage::Eyes && !screens_calling(),
			view_timer().resting, view_timer().finished);
		update_backlight(test_now);
		if (uint32_t(test_now - last_frame) >= FRAME_MS) {
			last_frame = test_now;
			render_frame(renderer, texture, window);
			if (limit_frames && ++rendered >= limit_frames) { running = false; }
		}
		SDL_Delay(1);
	}
	return 0;
}

/** @brief Print the supported command-line switches. @param program Program name. */
void usage(const char *program) {
	std::printf("Usage: %s [--frames N] [--page name] [--command line] [--screenshot path.ppm]\n", program);
	std::printf("  --frames N       render N frames then exit (headless smoke run)\n");
	std::printf("  --page name      start on a page: eyes clock route alerts weather\n");
	std::printf("                   settings desk bigclock overview timer device pong analogclock\n");
	std::printf("  --command line   run one USB console command at startup\n");
	std::printf("  --screenshot P   write the final canvas to a PPM image\n");
	std::printf("Mouse: click tap, drag swipe/scroll, hold 1.2s settings, wheel pages\n");
	std::printf("Keys:  1..6 pages, 0 desk, n alert, r route, p snapshot, q/ESC quit\n");
	std::printf("Terminal: type a USB command and press Enter (try: mood names, mood glow, help)\n");
	std::printf("Window: press Enter to type a command there instead; replies appear in the terminal\n");
}

/** @brief Parse the supported command-line switches. @param argc Count. @param argv Values. @return False on invalid input. */
bool parse_args(int argc, char **argv) {
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") { return false; }
		if (arg == "--frames" && i + 1 < argc) {
			limit_frames = uint32_t(std::strtoul(argv[++i], nullptr, 10));
			continue;
		}
		if (arg == "--screenshot" && i + 1 < argc) { shot_path = argv[++i]; continue; }
		if (arg == "--page" && i + 1 < argc) {
			std::string name = argv[++i];
			for (char &c : name) { c = char(std::tolower(char(c))); }
			if (!page_from_name(name, start_page)) {
				std::fprintf(stderr, "unknown page: %s\n", name.c_str());
				return false;
			}
			continue;
		}
		if (arg == "--command" && i + 1 < argc) { boot_command = argv[++i]; continue; }
		std::fprintf(stderr, "unknown option: %s\n", arg.c_str());
		return false;
	}
	return true;
}

#ifndef MOCHI_EMULATOR_INPUT_TEST
int main(int argc, char **argv) {
	if (!parse_args(argc, argv)) { usage(argv[0]); return 2; }
	std::srand(unsigned(std::time(nullptr)));
	if (SDL_Init(SDL_INIT_VIDEO) != 0) { std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
	SDL_Window *window = SDL_CreateWindow("Mochi LCD emulator", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, LCD_W * SCALE, LCD_H * SCALE, SDL_WINDOW_RESIZABLE);
	if (!window && limit_frames) {
		// Headless runs have no display server; the dummy driver still renders frames.
		SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
		SDL_Quit();
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			window = SDL_CreateWindow("Mochi LCD emulator", SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED, LCD_W * SCALE, LCD_H * SCALE, 0);
		}
	}
	if (!window) { std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) { renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE); }
	if (!renderer) { std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
	SDL_RenderSetLogicalSize(renderer, LCD_W, LCD_H);
	SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, LCD_W, LCD_H);
	if (!texture) { std::fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
	boot_firmware();
	if (!boot_command.empty()) { Serial.input += boot_command + "\n"; }
	if (start_page != page) { switch_screen(start_page, 0); }
	usage(argv[0]);
	SDL_StartTextInput();
	int status = event_loop(renderer, texture, window);
	if (limit_frames && !write_ppm(shot_path)) {
		std::fprintf(stderr, "screenshot failed: %s\n", shot_path.c_str());
		status = 1;
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return status;
}
#endif

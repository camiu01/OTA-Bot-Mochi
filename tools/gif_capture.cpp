/**
 * @file gif_capture.cpp
 * @brief Deterministic RGB24 capture of the real native eye renderer, without SDL.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

/** @brief Reject malformed or unbounded numeric arguments before allocating a canvas. */
static int number(const char *text, int low, int high) {
	char *end = nullptr;
	long value = std::strtol(text, &end, 10);
	if (!*text || *end || value < low || value > high) { throw std::runtime_error("Invalid numeric argument"); }
	return int(value);
}

/** @brief Match exactly the public style names. */
static EyeStyle style(const std::string &name) {
	if (name == "normal") { return NORMAL; }
	if (name == "mochi") { return MOCHI; }
	if (name == "bloub") { return BLOUB; }
	if (name == "trapez") { return TRAPEZ; }
	throw std::runtime_error("Unknown eye style");
}

/** @brief Convert a complete RGB565 canvas to one RGB24 frame. */
static void write_frame(std::ofstream &output, const Arduino_GFX &gfx) {
	for (uint16_t p : gfx.pixels) {
		unsigned char rgb[] = {uint8_t((p >> 11) * 255 / 31),
			uint8_t(((p >> 5) & 63) * 255 / 63), uint8_t((p & 31) * 255 / 31)};
		output.write(reinterpret_cast<const char *>(rgb), 3);
	}
	if (!output) { throw std::runtime_error("Cannot write RGB capture"); }
}

/** @brief Run the firmware's 33 ms animation cadence, sampling at the requested export FPS. */
static void capture(Arduino_GFX &gfx, const char *mood, const char *from,
		int frames, int fps, const char *path) {
	uint32_t base = 0;
	if (std::string(from) != "-") {
		if (!eyes_set_expression_by_name(from)) { throw std::runtime_error("Unknown source mood"); }
		for (test_now = 0; test_now < 6000; test_now += 33) { eyes_update(test_now); }
		test_now = base = 6000; eyes_update(test_now);
	}
	if (!eyes_set_expression_by_name(mood)) { throw std::runtime_error("Unknown mood"); }
	std::ofstream output(path, std::ios::binary);
	if (!output) { throw std::runtime_error("Cannot open RGB capture"); }
	uint32_t next = base;
	for (int i = 0; i < frames; ++i) {
		uint32_t target = base + uint32_t(i) * 1000 / uint32_t(fps);
		while (next <= target) { test_now = next; eyes_update(test_now); next += 33; }
		write_frame(output, gfx);
	}
	output.close();
	if (!output) { throw std::runtime_error("Cannot finish RGB capture"); }
}

/** @brief List canonical moods or capture one bounded animation for the Python GIF encoder. */
int main(int argc, char **argv) {
	try {
		if (argc == 2 && std::string(argv[1]) == "--list") {
			for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
				std::cout << eyes_expression_name(Expression(i)) << '\n';
			}
			return 0;
		}
		if (argc != 9) { throw std::runtime_error("Expected: mood style width height frames fps from output"); }
		int w = number(argv[3], 128, 480), h = number(argv[4], 128, 480);
		int frames = number(argv[5], 1, 900), fps = number(argv[6], 1, 30);
		EyeStyle selected = style(argv[2]);
		std::srand(1);
		if (!settings_init()) { throw std::runtime_error("Native settings initialization failed"); }
		auto p = settings_get();
		p.random_faces = p.imu_faces = p.shake = false;
		p.animation = 1;
		if (!settings_update(p)) { throw std::runtime_error("Native settings update failed"); }
		Arduino_GFX gfx(w, h);
		eyes_init(&gfx); eyes_set_style(selected);
		eyes_set_eye_colors(0, 0xFFFF, 0, 0xFFFF);
		capture(gfx, argv[1], argv[7], frames, fps, argv[8]);
		return 0;
	} catch (const std::exception &error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}

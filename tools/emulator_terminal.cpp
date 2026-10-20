/**
 * @file emulator_terminal.cpp
 * @brief Bounded, non-blocking console/pipe input for the desktop emulator.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "emulator.h"
#include <cstdio>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <poll.h>
#include <unistd.h>
#endif

namespace {
/** @brief Read one available stdin byte without waiting. @param echo True for a Windows console. @return Byte or -1. */
int read_byte(bool &echo) {
#ifdef _WIN32
	HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	if (GetConsoleMode(input, &mode)) {
		echo = true;
		if (!_kbhit()) { return -1; }
		int c = _getch();
		if (c == 0 || c == 224) { (void)_getch(); return -1; }
		return c;
	}
	DWORD available = 0, count = 0;
	if (!PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr) || !available) { return -1; }
	char c = 0;
	return ReadFile(input, &c, 1, &count, nullptr) && count == 1 ? uint8_t(c) : -1;
#else
	pollfd input{STDIN_FILENO, POLLIN, 0};
	if (poll(&input, 1, 0) <= 0 || !(input.revents & POLLIN)) { return -1; }
	char c = 0;
	(void)echo;
	return read(STDIN_FILENO, &c, 1) == 1 ? uint8_t(c) : -1;
#endif
}
}

void poll_terminal() {
	if (Serial.available() > 256) { return; }
	for (uint8_t i = 0; i < 64; ++i) {
		bool echo = false;
		int c = read_byte(echo);
		if (c < 0) { break; }
		if (echo) {
			if (c == '\b' || c == 127) { std::fputs("\b \b", stdout); }
			else if (c == '\r' || c == '\n') { std::fputc('\n', stdout); }
			else if (c >= 32) { std::fputc(c, stdout); }
			std::fflush(stdout);
		}
		terminal_character(char(c));
	}
}

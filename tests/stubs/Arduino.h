/**
 * @file Arduino.h
 * @brief Minimal native test substitute, not an Arduino compatibility layer.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <string>
#include <type_traits>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include "pwm.h"

inline uint32_t test_now = 0;
/** @brief Supply a controlled clock. @return Test timestamp. */
inline uint32_t millis() { return test_now; }
inline void delay(uint32_t ms) { test_now += ms; }
constexpr uint8_t INPUT_PULLUP = 2;
inline long random(long maximum) { return maximum > 0 ? std::rand() % maximum : 0; }
inline long random(long minimum, long maximum) { return minimum + random(maximum - minimum); }
/** @brief Controlled ADC samples for native battery tests; zero models an unfitted divider. */
inline int test_analog_read = 0;
inline int analogRead(int) { return test_analog_read; }

struct FakeSerial {
	std::string input, output;
	size_t position = 0;
	int available() const { return input.size() - position; }
	int read() { return available() ? (unsigned char)input[position++] : -1; }
	void println(const char *text) { output += text; output += '\n'; }
	void printf(const char *format, ...) {
		char buffer[512];
		va_list arguments;
		va_start(arguments, format);
		vsnprintf(buffer, sizeof(buffer), format, arguments);
		va_end(arguments);
		output += buffer;
	}
	void feed(const std::string &text) { input += text; }
	void flush() {}
	void clear() { input.clear(); output.clear(); position = 0; }
};
inline FakeSerial Serial;

/** @brief Observe restart requests without leaving the host process. */
struct FakeEsp {
	bool restarted = false;
	void restart() { restarted = true; }
};
inline FakeEsp ESP;

class String {
	std::string value;
public:
	String() = default;
	String(const char *text) : value(text ? text : "") {}
	String(const std::string &text) : value(text) {}
	template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
	String(T number) : value(std::to_string(number)) {}
	unsigned int length() const { return value.length(); }
	const char *c_str() const { return value.c_str(); }
	char operator[](unsigned int index) const { return value[index]; }
	void reserve(unsigned int length) { value.reserve(length); }
	String substring(unsigned int start, unsigned int end) const {
		return start >= value.length() ? String() : String(value.substr(start, end - start));
	}
	void replace(char from, char to) {
		for (char &c : value) { if (c == from) { c = to; } }
	}
	String &operator+=(char c) { value += c; return *this; }
	friend String operator+(const String &a, const String &b) { return a.value + b.value; }
	friend bool operator==(const String &a, const String &b) { return a.value == b.value; }
};

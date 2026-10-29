/**
 * @file ancs_test.cpp
 * @brief Native ANCS packet and fragmentation regression checks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ancs/ancs_parser.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

/** @brief Append one ANCS attribute to a response. */
static void attribute(std::vector<uint8_t> &response, uint8_t id, const std::string &value) {
	response.push_back(id);
	response.push_back(uint8_t(value.size()));
	response.push_back(uint8_t(value.size() >> 8));
	response.insert(response.end(), value.begin(), value.end());
}

/** @brief Verify bounded, fragmented, malformed, and reset behavior. @return Process status. */
int main() {
	AncsParser parser;
	uint8_t event[] = {0, 0, 4, 1, 42, 0, 0, 0};
	assert(!parser.event(event, 7));
	assert(parser.event(event, sizeof(event)));
	std::vector<uint8_t> response = {0, 42, 0, 0, 0};
	attribute(response, 0, "com.example.app");
	attribute(response, 1, "Title");
	attribute(response, 3, std::string(1100, 'x'));
	attribute(response, 5, "20261029T120000");
	assert(!parser.append(response.data(), 7));
	assert(!parser.append(response.data() + 7, response.size() - 7));
	assert(!parser.pending());
	assert(parser.event(event, sizeof(event)));
	response.clear(); response = {0, 42, 0, 0, 0};
	attribute(response, 0, "com.example.app");
	attribute(response, 1, "Title");
	attribute(response, 3, std::string(1024, 'x'));
	attribute(response, 5, "20261029T120000");
	for (size_t at = 0; at < response.size(); at += 17) {
		size_t count = response.size() - at < 17 ? response.size() - at : 17;
		bool done = parser.append(response.data() + at, count);
		if (at + count == response.size()) { assert(done); }
	}
	assert(parser.alert().uid == 42 && parser.alert().title == "Title");
	assert(parser.alert().message.size() == 1024);
	parser.set_app("Example");
	assert(parser.alert().app == "Example");
	uint8_t removed[] = {2, 0, 4, 1, 42, 0, 0, 0};
	assert(!parser.event(removed, sizeof(removed)));
	assert(parser.alert().app.empty());
	assert(parser.event(event, sizeof(event)));
	response[1] = 43;
	assert(!parser.append(response.data(), response.size()));
	assert(!parser.pending());
	parser.reset();
	assert(!parser.pending() && parser.alert().title.empty());
	assert(!parser.append(response.data(), response.size()));
}

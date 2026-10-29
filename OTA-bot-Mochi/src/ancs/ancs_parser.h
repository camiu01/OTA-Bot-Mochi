/**
 * @file ancs_parser.h
 * @brief Bounded decoding of Apple notification events and attribute streams.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

struct AncsAlert {
	uint32_t uid = 0;
	std::string app;
	std::string title;
	std::string message;
	std::string date;
};

class AncsParser {
public:
	/** @brief Accept a notification source event. @param data Event bytes. @param length Byte count. @return True for a new or modified alert. */
	bool event(const uint8_t *data, size_t length);
	/** @brief Append a data source fragment. @param data Fragment bytes. @param length Byte count. @return True when all requested attributes are complete. */
	bool append(const uint8_t *data, size_t length);
	/** @brief Reset pending data after disconnect or failure. */
	void reset();
	/** @brief Read the completed alert. @return Bounded alert. */
	const AncsAlert &alert() const { return alert_; }
	/** @brief Replace a bundle ID with the authorized iOS display name. @param name Bounded app label. */
	void set_app(const std::string &name) { alert_.app = name.substr(0, 64); }
	/** @brief Check whether an attribute request is pending. @return True when pending. */
	bool pending() const { return pending_; }
private:
	AncsAlert alert_;
	std::string bytes_;
	bool pending_ = false;
	uint8_t seen_ = 0;
};

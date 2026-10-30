/**
 * @file SD_MMC.h
 * @brief In-memory, read-only SD fake with mount, removal and short-read controls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>
#define FILE_READ "r"

/** @brief Seekable immutable file fixture matching the Arduino File read API. */
class File {
public:
	std::shared_ptr<std::vector<uint8_t>> data;
	size_t offset = 0;
	inline static bool short_read = false;
	/** @brief Read file validity. @return Open. */
	explicit operator bool() const { return bool(data); }
	/** @brief Read fixture length. @return Bytes. */
	size_t size() const { return data ? data->size() : 0; }
	/** @brief Seek inside the fixture. @param at Byte offset. @return Valid. */
	bool seek(uint32_t at) { if (!data || at > data->size()) { return false; } offset = at; return true; }
	/** @brief Read bounded bytes. @param out Destination. @param count Requested count. @return Bytes read. */
	size_t read(uint8_t *out, size_t count) {
		if (!data || short_read) { return 0; }
		size_t available = std::min(count, data->size() - offset);
		memcpy(out, data->data() + offset, available); offset += available; return available;
	}
	/** @brief Drop the open reference. */
	void close() { data.reset(); offset = 0; }
};

/** @brief Fake mount and fixed-path lookup, never writing or formatting fixtures. */
class TestSdMmc {
public:
	bool present = false;
	std::string last_path;
	unsigned mounts = 0, opens = 0, unmounts = 0;
	int clk = -1, cmd = -1, data = -1;
	std::map<std::string, std::vector<uint8_t>> files;
	/** @brief Retain wiring for assertions. @param clock CLK. @param command CMD. @param input D0. @return Accepted. */
	bool setPins(int clock, int command, int input) { clk = clock; cmd = command; data = input; return true; }
	/** @brief Check one-bit, non-formatting mount semantics. @param path Mountpoint. @param one_bit Width. @param format Formatting. @return Present and valid options. */
	bool begin(const char *path, bool one_bit, bool format) {
		++mounts; return present && one_bit && !format && std::string(path) == "/sdcard";
	}
	/** @brief Count unmounts. */
	void end() { ++unmounts; }
	/** @brief Open a read-only fixture. @param path Fixed cue path. @param mode Read mode. @return File. */
	File open(const char *path, const char *mode) {
		++opens; last_path = path; File file;
		auto found = files.find(path);
		if (present && std::string(mode) == "r" && found != files.end()) { file.data = std::make_shared<std::vector<uint8_t>>(found->second); }
		return file;
	}
};
inline TestSdMmc SD_MMC;

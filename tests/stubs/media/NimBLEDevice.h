/**
 * @file NimBLEDevice.h
 * @brief Controlled GATT transport for native Apple Media Service tests only.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

/** @brief Fake characteristic recording subscriptions, writes and extended values. */
class NimBLERemoteCharacteristic {
public:
	using notify_callback = std::function<void(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool)>;
	bool subscribe_ok = true, write_ok = true;
	std::vector<std::vector<uint8_t>> writes;
	std::vector<uint8_t> value;
	notify_callback callback;
	/** @brief Record subscription. @param notifications Mode. @param cb Handler. @return Configured result. */
	bool subscribe(bool notifications, notify_callback cb) { (void)notifications; callback = cb; return subscribe_ok; }
	/** @brief Record outgoing bytes. @param data Bytes. @param size Count. @param response Mode. @return Configured result. */
	bool writeValue(const uint8_t *data, size_t size, bool response) {
		(void)response; writes.emplace_back(data, data + size); return write_ok;
	}
	/** @brief Return controlled extended metadata. @return Bytes. */
	std::vector<uint8_t> readValue() { return value; }
};

/** @brief Fake service holding the named AMS characteristics. */
class NimBLERemoteService {
public:
	std::map<std::string, NimBLERemoteCharacteristic *> characteristics;
	/** @brief Find a characteristic. @param uuid Identifier. @return Characteristic or null. */
	NimBLERemoteCharacteristic *getCharacteristic(const char *uuid) {
		auto found = characteristics.find(uuid);
		return found == characteristics.end() ? nullptr : found->second;
	}
};

/** @brief Fake paired connection exposing an optional service. */
class NimBLEClient {
public:
	bool connected = true, secure = true;
	NimBLERemoteService *service = nullptr;
	/** @brief Read connection status. @return Connected. */
	bool isConnected() const { return connected; }
	/** @brief Model encryption failure. @return Secure. */
	bool secureConnection() const { return secure; }
	/** @brief Return the configured AMS service. @param uuid Identifier. @return Service. */
	NimBLERemoteService *getService(const char *uuid) { (void)uuid; return service; }
};

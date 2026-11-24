/**
 * @file NimBLEDevice.h
 * @brief Declaration-only NimBLE central API for the obd_ble.cpp host syntax check.
 *
 * Mirrors the NimBLE-Arduino 2.x signatures that obd_ble.cpp uses so a renamed
 * or retyped call fails natively. It is never linked or executed.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class NimBLEUUID {
public:
	NimBLEUUID(const char *text);
	NimBLEUUID(uint16_t value);
	bool operator==(const NimBLEUUID &other) const;
};

class NimBLEAddress {
public:
	NimBLEAddress();
	std::string toString() const;
};

class NimBLERemoteCharacteristic {
public:
	using notify_callback = void (*)(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool);
	bool canNotify() const;
	bool canIndicate() const;
	bool canWrite() const;
	bool canWriteNoResponse() const;
	bool subscribe(bool notifications, notify_callback callback);
	bool writeValue(const uint8_t *data, size_t length, bool response);
};

class NimBLERemoteService {
public:
	const std::vector<NimBLERemoteCharacteristic *> &getCharacteristics(bool refresh);
	const NimBLEUUID &getUUID() const;
};

class NimBLEClient {
public:
	void setConnectTimeout(uint32_t ms);
	bool connect(const NimBLEAddress &address);
	bool disconnect();
	bool isConnected() const;
	NimBLERemoteService *getService(const NimBLEUUID &uuid);
	const std::vector<NimBLERemoteService *> &getServices(bool refresh);
};

class NimBLEAdvertisedDevice {
public:
	std::string getName() const;
	NimBLEAddress getAddress() const;
	int getRSSI() const;
	bool isAdvertisingService(const NimBLEUUID &uuid) const;
};

class NimBLEScanResults {
public:
	int getCount() const;
	const NimBLEAdvertisedDevice *getDevice(uint32_t index) const;
};

class NimBLEScan {
public:
	void setActiveScan(bool active);
	NimBLEScanResults getResults(uint32_t duration, bool continued);
	void clearResults();
};

class NimBLEDevice {
public:
	static NimBLEScan *getScan();
	static NimBLEClient *createClient();
};

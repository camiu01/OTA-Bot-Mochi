/**
 * @file ams_service.h
 * @brief Worker-owned Apple Media Service client sharing the ANCS connection.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <NimBLEDevice.h>
#include "../ui/music_protocol.h"

/** @brief Own AMS pointers exclusively on the existing iPhone worker task. */
class AmsService {
public:
	/** @brief Discover and subscribe after pairing. @param client Connected iPhone. @param entity Metadata callback. @param commands Capability callback. @return Ready. */
	bool discover(NimBLEClient *client, NimBLERemoteCharacteristic::notify_callback entity, NimBLERemoteCharacteristic::notify_callback commands);
	/** @brief Clear pointers and metadata on disconnect or service changes. */
	void reset();
	/** @brief Decode queued data, fetching truncated attributes on this worker. @param data Packet. @param size Bytes. @param commands Capability list instead of metadata. */
	void receive(const uint8_t *data, size_t size, bool commands);
	/** @brief Forward one queued UI command; report failed writes as unavailable. */
	void poll();
	/** @brief Expose discovery state. @return Subscriptions installed. */
	bool ready() const { return _remote != nullptr; }
private:
	NimBLERemoteCharacteristic *_remote = nullptr, *_attribute = nullptr;
	UiMusic _music;
};

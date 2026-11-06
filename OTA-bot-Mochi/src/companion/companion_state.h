/**
 * @file companion_state.h
 * @brief Bounded Chronos snapshots shared by BLE callbacks, the main task and phone tools.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <ChronosESP32.h>

/** @brief Minute-accurate phone schedule; intervals are in minutes. */
struct CompanionWindow {
	uint16_t start = 0, end = 0, interval = 0;
	bool enabled = false;
};
/** @brief Small owned contact; excess phone contacts are explicitly excluded from the local cache. */
struct CompanionContact { char name[41] = {}, number[25] = {}; };
/** @brief Owned validated hourly weather, independent of library mutation. */
struct CompanionHour {
	int16_t day = 0, wind = 0;
	int8_t temp = 0;
	uint8_t hour = 0, icon = 0, humidity = 0, uv = 0;
};
/** @brief Metadata copied without copying the contact/QR payload buffers. */
struct CompanionSnapshot {
	bool connected = false, camera = false, phone_charging = false;
	uint8_t battery = 255, language = 255, contacts = 0, total_contacts = 0;
	Alarm alarms[8] = {};
	uint8_t alarm_mask = 0;
	CompanionWindow quiet, sleep, reminders[2];
	uint32_t revision = 0, finds = 0, forecast_mask = 0;
	uint16_t qr_mask = 0;
};
/** @brief Queued commands are consumed only by the main task. */
enum class CompanionAction : uint8_t { None, FindPhone, StopFind, Camera };
/** @brief Create the static mutex before BLE starts. @return Ready. */
bool companion_init();
/** @brief Clear transient phone capabilities on disconnect. @param connected Link state. */
void companion_connection(bool connected);
/** @brief Copy supported finalized Chronos packets under a mutex. @param watch Source. @param config Packet. @param a Parameter. @param b Parameter. */
void companion_config(ChronosESP32 &watch, Config config, uint32_t a, uint32_t b);
/** @brief Decode bounded hourly packets absent from the library's configuration callbacks. @param data Packet. @param length Bytes. @param day Local zero-based day of year. */
void companion_data(const uint8_t *data, size_t length, int day);
/** @brief Disable a fired one-shot in the owned session copy. @param index Alarm slot. */
void companion_alarm_used(uint8_t index);
/** @brief Copy small metadata. @return Owned snapshot. */
CompanionSnapshot companion_snapshot();
/** @brief Copy one bounded contact. @param index Cache index. @return Contact or empty. */
CompanionContact companion_contact(uint8_t index);
/** @brief Copy one validated forecast slot. @param index Hour index. @return Hour or empty. */
CompanionHour companion_hour(uint8_t index);
/** @brief Copy one complete QR payload; long payloads are rejected, never truncated. @param index Slot. @return Payload or empty. */
String companion_qr(uint8_t index);
/** @brief Queue one connected action, rejecting duplicate pending work. @param action Requested action. @return Queued. */
bool companion_request(CompanionAction action);
/** @brief Consume queued work once. @return Action or None. */
CompanionAction companion_take();

/** @brief Native OTA transport state, not a radio simulation. */
#pragma once
#include <Arduino.h>
struct IPAddress { IPAddress(int, int, int, int) {} };
constexpr int WIFI_OFF = 0, WIFI_AP = 2;
struct FakeWiFi {
	int mode_value = WIFI_OFF;
	bool ready = true;
	bool persistent_value = true;
	void persistent(bool value) { persistent_value = value; }
	bool mode(int value) { mode_value = value; return ready; }
	bool softAPConfig(IPAddress, IPAddress, IPAddress) { return ready; }
	bool softAP(const char *, const char *, int, bool, int) { return ready; }
	void softAPdisconnect(bool) {}
};
inline FakeWiFi WiFi;

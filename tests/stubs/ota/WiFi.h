/** @brief Native OTA transport state, not a radio simulation. */
#pragma once
#include <Arduino.h>
#include <vector>
struct IPAddress {
	int a, b, c, d;
	IPAddress(int one, int two, int three, int four) : a(one), b(two), c(three), d(four) {}
	String toString() const { return String(a) + "." + String(b) + "." + String(c) + "." + String(d); }
};
constexpr int WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3, WL_CONNECTED = 3;
constexpr int WIFI_SCAN_RUNNING = -1, WIFI_SCAN_FAILED = -2;
struct FakeWiFi {
	int scan_result = WIFI_SCAN_FAILED, scan_starts = 0;
	std::vector<String> networks;
	int scanNetworks(bool, bool, bool, uint32_t) { ++scan_starts; scan_result = WIFI_SCAN_RUNNING; return scan_result; }
	int scanComplete() { return scan_result; }
	void scanDelete() { scan_result = WIFI_SCAN_FAILED; networks.clear(); }
	String SSID(int index) { return networks.at(index); }
	int mode_value = WIFI_OFF;
	bool ready = true;
	bool persistent_value = true;
	bool reconnect = true;
	int connection = 0, station_begins = 0, station_disconnects = 0;
	IPAddress ip{192, 168, 1, 42};
	IPAddress localIP() { return ip; }
	void setAutoReconnect(bool value) { reconnect = value; }
	void begin(const char *, const char *) { ++station_begins; }
	int status() { return connection; }
	void disconnect(bool, bool) { ++station_disconnects; connection = 0; }
	void persistent(bool value) { persistent_value = value; }
	bool mode(int value) { mode_value = value; return ready; }
	bool softAPConfig(IPAddress, IPAddress, IPAddress) { return ready; }
	bool softAP(const char *, const char *, int, bool, int) { return ready; }
	void softAPdisconnect(bool) {}
};
inline FakeWiFi WiFi;

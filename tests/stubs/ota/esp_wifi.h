/** @brief Native scan cancellation observable, not radio behavior. */
#pragma once
inline int test_scan_stops = 0;
inline int esp_wifi_scan_stop() { ++test_scan_stops; return 0; }

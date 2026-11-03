/** @file esp_bt.h @brief Observable Bluetooth entropy readiness for native tests. */
#pragma once
enum esp_bt_controller_status_t { ESP_BT_CONTROLLER_STATUS_IDLE, ESP_BT_CONTROLLER_STATUS_ENABLED };
inline esp_bt_controller_status_t test_bt_status = ESP_BT_CONTROLLER_STATUS_ENABLED;
inline esp_bt_controller_status_t esp_bt_controller_get_status() { return test_bt_status; }

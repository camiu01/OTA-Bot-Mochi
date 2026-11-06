/** @file companion_service.h @brief Main-task Chronos actions, alarms, reminders and schedule application. */
#pragma once
#include "companion_state.h"
/** @brief Service validated phone data without blocking or saving profiles. @param watch Transport. @param now Milliseconds. @param clock_valid Trusted local time. */
void companion_poll(ChronosESP32 &watch, uint32_t now, bool clock_valid);
/** @brief Report the current camera action result to the UI. @return 0 idle, 1 sent, 2 rejected. */
uint8_t companion_camera_result();
/** @brief Report the bounded local find-phone session. @return Finding. */
bool companion_finding();

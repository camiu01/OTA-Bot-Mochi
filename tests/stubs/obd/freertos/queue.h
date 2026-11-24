/**
 * @file queue.h
 * @brief Declaration-only FreeRTOS queue API for the obd_ble.cpp host syntax check.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <freertos/FreeRTOS.h>
using QueueHandle_t = void *;
#define pdTRUE 1
QueueHandle_t xQueueCreate(unsigned length, unsigned item_size);
int xQueueSend(QueueHandle_t queue, const void *item, unsigned ticks);
int xQueueReceive(QueueHandle_t queue, void *item, unsigned ticks);

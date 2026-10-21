/**
 * @file i2s_std.h
 * @brief Native TX-only I2S fake with injectable allocation and DMA failures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino.h>
#include <vector>
#include <cstring>
using esp_err_t = int;
using gpio_num_t = int;
constexpr int ESP_OK = 0, I2S_NUM_AUTO = -1, I2S_ROLE_MASTER = 1;
constexpr int I2S_DATA_BIT_WIDTH_16BIT = 16, I2S_SLOT_MODE_STEREO = 2, I2S_GPIO_UNUSED = -1;
struct i2s_chan_config_t { int id, role, dma_desc_num, dma_frame_num; bool auto_clear; };
struct i2s_std_clk_config_t { int sample_rate_hz, mclk_multiple; };
struct i2s_std_slot_config_t { int data_bit_width, slot_mode; };
struct i2s_std_config_t {
	i2s_std_clk_config_t clk_cfg;
	i2s_std_slot_config_t slot_cfg;
	struct { int mclk, bclk, ws, dout, din; } gpio_cfg;
};
#define I2S_CHANNEL_DEFAULT_CONFIG(id, role) {id, role, 6, 240, false}
#define I2S_STD_CLK_DEFAULT_CONFIG(rate) {rate, 256}
#define I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits, mode) {bits, mode}
struct TestAudioYield {};
using i2s_chan_handle_t = int *;
inline int test_i2s_channel = 0, test_i2s_failure = 0, test_i2s_allocated = 0;
inline int test_i2s_remaining = 1;
inline bool test_i2s_short_write = false;
inline i2s_chan_config_t test_i2s_config{};
inline i2s_std_config_t test_i2s_standard{};
inline std::vector<int16_t> test_i2s_samples;
inline std::vector<int16_t> test_i2s_history;
inline void (*test_i2s_hook)() = nullptr;
inline int i2s_new_channel(const i2s_chan_config_t *config, i2s_chan_handle_t *tx, i2s_chan_handle_t *rx) {
	if (test_i2s_failure == 1 || rx) { return -1; }
	test_i2s_config = *config;
	*tx = &test_i2s_channel;
	++test_i2s_allocated;
	return ESP_OK;
}
inline int i2s_channel_init_std_mode(i2s_chan_handle_t, const i2s_std_config_t *config) {
	test_i2s_standard = *config;
	return test_i2s_failure == 2 ? -1 : ESP_OK;
}
inline int i2s_channel_enable(i2s_chan_handle_t) { return test_i2s_failure == 3 ? -1 : ESP_OK; }
inline int i2s_channel_disable(i2s_chan_handle_t) { return ESP_OK; }
inline int i2s_del_channel(i2s_chan_handle_t) { --test_i2s_allocated; return ESP_OK; }
inline int i2s_channel_write(i2s_chan_handle_t, const void *data, size_t size, size_t *written, uint32_t timeout) {
	if (test_i2s_remaining-- <= 0) { throw TestAudioYield{}; }
	if (timeout != 20 || test_i2s_failure == 4) { return -1; }
	test_i2s_samples.resize(size / sizeof(int16_t));
	std::memcpy(test_i2s_samples.data(), data, size);
	test_i2s_history.insert(test_i2s_history.end(), test_i2s_samples.begin(), test_i2s_samples.end());
	*written = test_i2s_short_write ? size / 2 : size;
	test_now += 5;
	if (test_i2s_hook) { test_i2s_hook(); }
	return ESP_OK;
}

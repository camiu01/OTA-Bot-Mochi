/**
 * @file es8311_output.cpp
 * @brief Shared-I2C ES8311 control and bounded, playback-only I2S worker with opt-in SD WAV cues.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../../pin_config.h"
#if defined(MOCHI_BOARD_AMOLED_18)
#include "es8311_output.h"
#include "pcm_tone.h"
#include "sd_audio.h"
#include "../bus/i2c_bus.h"
#include <Arduino.h>
#include <atomic>
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace {
constexpr uint8_t CODEC_ADDRESS = 0x18;
i2s_chan_handle_t channel = nullptr;
bool initialized = false, amplifier_owned = false;
struct AudioCommand {
	SoundNote notes[5] = {};
	uint8_t count = 0, level = 0, sd_cue = 255;
	uint32_t revision = 0, updated = 0;
	bool sequence = false;
};
StaticSemaphore_t command_storage;
SemaphoreHandle_t command_lock = nullptr;
AudioCommand command;
std::atomic<bool> transport_failed{false}, playing{false};

/** @brief Publish under the same lock used by the worker's amplifier updates. @param next Copied command. */
void publish(AudioCommand next) {
	xSemaphoreTake(command_lock, portMAX_DELAY);
	next.revision = command.revision + 1;
	command = next;
	playing.store(next.sequence);
	digitalWrite(AUDIO_PA, next.level ? HIGH : LOW);
	xSemaphoreGive(command_lock);
}

/** @brief Write one codec register on the main task's existing bus. @param reg Register. @param value Value. @return Acknowledged. */
bool codec_write(uint8_t reg, uint8_t value) {
	TwoWire &wire = i2c_bus();
	wire.beginTransmission(CODEC_ADDRESS);
	wire.write(reg);
	wire.write(value);
	if (wire.endTransmission() == 0) { return true; }
	Serial.printf("ES8311 I2C write failed at register 0x%02X\n", unsigned(reg));
	return false;
}

/** @brief Apply the Waveshare 16 kHz/4.096 MHz MCLK, slave I2S/16-bit sequence. @return Success. */
bool codec_init() {
	// Fixed-rate subset of Waveshare's 15_ES8311 demo (Espressif register sequence):
	// https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/tree/78e13f852929c2ab4f9d5e0ad1c50ea378dbf2b4/examples/arduino-v2/examples/15_ES8311
	static constexpr uint8_t registers[][2] = {
		{0x00, 0x1F}, {0x00, 0x00}, {0x00, 0x80}, // Reset, power on, slave.
		{0x01, 0x3F}, {0x02, 0x00}, {0x03, 0x10}, {0x04, 0x10},
		{0x05, 0x00}, {0x06, 0x03}, {0x07, 0x00}, {0x08, 0xFF},
		{0x09, 0x0C}, {0x0A, 0x0C}, // Philips I2S, 16-bit input/output.
		{0x0D, 0x01}, {0x0E, 0x02}, {0x12, 0x00}, {0x13, 0x10},
		{0x1C, 0x6A}, {0x37, 0x08}, {0x32, 0xD8}, {0x31, 0x00}
	};
	for (const auto &item : registers) {
		if (!codec_write(item[0], item[1])) { return false; }
	}
	return true;
}

/** @brief Generate bounded tone/SD PCM and leave five silent blocks for the four DMA descriptors to drain. @param out Stereo block. @param next Coherent command. @return Amplifier needed. */
bool render_block(int16_t *out, const AudioCommand &next) {
	static PcmSequence sequence;
	static PcmTone oscillator;
	static SdAudio sd;
	static bool sd_selected = false;
	static uint32_t revision = 0;
	static uint8_t drain = 0;
	if (revision != next.revision) {
		revision = next.revision; drain = 0;
		sd_selected = false;
		if (next.sequence && next.sd_cue != 255) { sd_selected = sd.start(next.sd_cue); }
		else { sd.release(); }
		sequence.start(next.notes, next.sequence ? next.count : 0, next.level);
	}
	bool audible = next.sequence ? (sd_selected ? sd.playing() : sequence.playing()) :
		(next.notes[0].hz && next.level && uint32_t(millis() - next.updated) <= 100);
	if (sd_selected) { sd.render(out, 80, next.level); }
	else if (next.sequence) { sequence.render(out, 80); }
	else { oscillator.render(out, 80, audible ? next.notes[0].hz : 0, next.level); }
	bool enabled = audible || drain;
	if (audible) { drain = 5; }
	else if (drain) { --drain; }
	return enabled;
}

/** @brief Own PCM/DMA and amplifier completion; never access I2C or preferences here. @param unused Unused. */
void audio_task(void *) {
	int16_t samples[160];
	for (;;) {
		if (transport_failed.load()) { vTaskDelay(pdMS_TO_TICKS(10)); continue; }
		xSemaphoreTake(command_lock, portMAX_DELAY);
		AudioCommand next = command;
		xSemaphoreGive(command_lock);
		bool enabled = render_block(samples, next);
		xSemaphoreTake(command_lock, portMAX_DELAY);
		if (command.revision == next.revision) {
			digitalWrite(AUDIO_PA, enabled ? HIGH : LOW);
			playing.store(next.sequence && enabled);
		}
		xSemaphoreGive(command_lock);
		size_t written = 0;
		esp_err_t error = i2s_channel_write(channel, samples, sizeof(samples), &written, 20);
		if (error != ESP_OK || written != sizeof(samples)) {
			xSemaphoreTake(command_lock, portMAX_DELAY);
			transport_failed.store(true); playing.store(false);
			digitalWrite(AUDIO_PA, LOW);
			xSemaphoreGive(command_lock);
		}
	}
}

/** @brief Create the TX-only standard channel; never claim the microphone data pin. @return Success. */
bool start_i2s() {
	i2s_chan_config_t config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	config.dma_desc_num = 4;
	config.dma_frame_num = 80;
	config.auto_clear = true;
	if (i2s_new_channel(&config, &channel, nullptr) != ESP_OK) { return false; }
	i2s_std_config_t standard = {};
	standard.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000);
	standard.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
	standard.gpio_cfg.mclk = gpio_num_t(I2S_MCK_IO);
	standard.gpio_cfg.bclk = gpio_num_t(I2S_BCK_IO);
	standard.gpio_cfg.ws = gpio_num_t(I2S_WS_IO);
	standard.gpio_cfg.dout = gpio_num_t(I2S_DO_IO);
	standard.gpio_cfg.din = I2S_GPIO_UNUSED;
	return i2s_channel_init_std_mode(channel, &standard) == ESP_OK &&
		i2s_channel_enable(channel) == ESP_OK;
}

/** @brief Release a partially initialized channel before any worker owns it. */
void cleanup() {
	if (!channel) { return; }
	// Disabling a not-yet-enabled channel is harmless during partial-init cleanup.
	i2s_channel_disable(channel);
	i2s_del_channel(channel);
	channel = nullptr;
}
}

bool es8311_begin() {
	if (initialized) { transport_failed.store(false); return true; }
	if (!command_lock) { command_lock = xSemaphoreCreateMutexStatic(&command_storage); }
	if (!command_lock) { return false; }
	pinMode(AUDIO_PA, OUTPUT);
	digitalWrite(AUDIO_PA, LOW);
	amplifier_owned = true;
	if (!i2c_bus_begin(IIC_SDA, IIC_SCL, 400000) || !start_i2s() || !codec_init()) {
		cleanup();
		Serial.println("ES8311 initialization failed; amplifier muted");
		return false;
	}
	if (xTaskCreate(audio_task, "mochi-audio", 6144, nullptr, 2, nullptr) != pdPASS) {
		cleanup();
		Serial.println("ES8311 PCM task allocation failed; amplifier muted");
		return false;
	}
	initialized = true;
	Serial.println("ES8311 ready: I2S playback, 16000 Hz");
	return true;
}

bool es8311_write(uint16_t hz, uint8_t percent) {
	if (!initialized || transport_failed.load() || percent > 30 ||
		(hz && (hz < 300 || hz > 4000))) { return false; }
	AudioCommand next;
	next.notes[0] = {hz, 0};
	next.level = hz ? percent : 0;
	next.updated = millis();
	publish(next);
	return true;
}

bool es8311_play(const SoundNote *notes, uint8_t count, uint8_t percent, uint8_t sd_cue) {
	if (!initialized || transport_failed.load() || !PcmSequence::valid(notes, count, percent)) { return false; }
	AudioCommand next;
	for (uint8_t i = 0; i < count; ++i) { next.notes[i] = notes[i]; }
	next.count = count; next.level = percent; next.sequence = true; next.sd_cue = sd_cue;
	publish(next);
	return true;
}

void es8311_end() {
	if (amplifier_owned && command_lock) { publish(AudioCommand{}); }
}

bool es8311_playing() { return playing.load(); }
bool es8311_healthy() { return !transport_failed.load(); }
#endif

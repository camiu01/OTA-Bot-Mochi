/** @file page_slide.h @brief Bounded horizontal transitions between complete RGB565 frames. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include "../../pin_config.h"

class PageSlide {
public:
	using Allocate = void *(*)(size_t);
	static constexpr uint32_t Duration = 200;
	static constexpr bool Supported = MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448;

	explicit PageSlide(Allocate allocate) : allocate_(allocate) {}
	~PageSlide() { std::free(frames_); }
	PageSlide(const PageSlide &) = delete;
	PageSlide &operator=(const PageSlide &) = delete;

	/** @brief Discard the current frame after an external screen or rotation change. */
	void reset() { ready_ = pending_ = playing_ = false; }

	/** @brief Capture the currently visible frame before the page switch clears the canvas. */
	void request(const uint16_t *pixels, int8_t direction) {
		if (!Supported || !ready_ || !pixels || !direction) { return; }
		if (!frames_) { frames_ = static_cast<uint16_t *>(allocate_(2 * Bytes)); }
		if (!frames_) { return; }
		if (!pending_) { std::memcpy(frames_, pixels, Bytes); }
		direction_ = direction;
		pending_ = true;
		playing_ = false;
	}

	/** @brief Capture the newly rendered page and compose the first animation frame. */
	void after_draw(uint16_t *pixels, uint32_t now, bool flipped = false) {
		if (!Supported || !pixels) { return; }
		ready_ = true;
		if (!pending_) { return; }
		std::memcpy(frames_ + Pixels, pixels, Bytes);
		pending_ = false;
		playing_ = true;
		started_ = now;
		paint(pixels, now, flipped);
	}

	/** @brief Paint an active transition instead of drawing over it with live page data. */
	bool paint(uint16_t *pixels, uint32_t now, bool flipped = false) {
		if (!playing_ || !pixels) { return false; }
		const uint32_t age = now - started_;
		if (age >= Duration) {
			std::memcpy(pixels, frames_ + Pixels, Bytes);
			playing_ = false;
			return true;
		}
		const uint32_t t = age * 256 / Duration;
		const uint32_t eased = (t * t * (768 - 2 * t) + 32768) >> 16;
		const int32_t steps = (eased * LCD_WIDTH + 128) >> 8;
		compose(pixels, steps, (direction_ > 0) != flipped);
		return true;
	}

private:
	static constexpr size_t Pixels = size_t(LCD_WIDTH) * LCD_HEIGHT;
	static constexpr size_t Bytes = Pixels * sizeof(uint16_t);
	Allocate allocate_;
	uint16_t *frames_ = nullptr;
	bool ready_ = false, pending_ = false, playing_ = false;
	int8_t direction_ = 1;
	uint32_t started_ = 0;

	/** @brief Join the visible parts of the two source frames without resampling. */
	void compose(uint16_t *pixels, int32_t steps, bool left) const {
		const size_t shift = size_t(steps), visible = LCD_WIDTH - shift;
		for (size_t offset = 0; offset < Pixels; offset += LCD_WIDTH) {
			const uint16_t *from = frames_ + offset;
			const uint16_t *to = from + Pixels;
			uint16_t *row = pixels + offset;
			if (left) {
				std::memcpy(row, from + shift, visible * sizeof(uint16_t));
				std::memcpy(row + visible, to, shift * sizeof(uint16_t));
			} else {
				std::memcpy(row, to + visible, shift * sizeof(uint16_t));
				std::memcpy(row + shift, from, visible * sizeof(uint16_t));
			}
		}
	}
};

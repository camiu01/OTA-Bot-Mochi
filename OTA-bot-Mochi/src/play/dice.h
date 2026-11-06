/**
 * @file dice.h
 * @brief Bounded RAM-only D&D dice pools, injected random source and paginated results.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

class DicePool {
public:
	static constexpr uint8_t MAX_DICE = 100, PER_PAGE = 12;
	/** @brief Selected homogeneous pool size. @return 1..100. */
	uint8_t count() const { return _count; }
	/** @brief Standard D&D die, including percentile D100. @return Faces. */
	uint8_t sides() const {
		static constexpr uint8_t types[] = {4, 6, 8, 10, 12, 20, 100};
		return types[_type];
	}
	/** @brief Completed roll available. @return Rolled. */
	bool rolled() const { return _rolled; }
	/** @brief Sum without eight-bit overflow. @return 0 pending, otherwise count..count*sides. */
	uint16_t total() const { return _total; }
	/** @brief Read a result without exposing stale values after configuration changes. @param index Die index. @return Result, or zero. */
	uint8_t value(uint8_t index) const { return _rolled && index < _count ? _values[index] : 0; }
	/** @brief Current result page, zero-based. @return Page. */
	uint8_t page() const { return _page; }
	/** @brief Number of pages for the selected count. @return 1..9. */
	uint8_t pages() const { return (_count + PER_PAGE - 1) / PER_PAGE; }
	/** @brief Clamp count edits and discard results only when selection changes. @param delta Signed adjustment. @return Changed. */
	bool adjust_count(int16_t delta) {
		int32_t next = int32_t(_count) + delta;
		if (next < 1) { next = 1; }
		if (next > MAX_DICE) { next = MAX_DICE; }
		if (next == _count) { return false; }
		_count = uint8_t(next); clear(); return true;
	}
	/** @brief Cycle the standard types in either direction, clearing old results. @param step Positive next, negative previous. */
	void cycle_type(int8_t step) {
		if (!step) { return; }
		_type = (_type + (step > 0 ? 1 : 6)) % 7; clear();
	}
	/** @brief Roll each die exactly once; rendering never consumes randomness. @tparam Random Bounded random callable. @param draw Returns 0..limit-1 for the supplied limit. */
	template<class Random> void roll(Random draw) {
		_total = 0; _page = 0;
		for (uint8_t i = 0; i < _count; ++i) {
			_values[i] = uint8_t(1 + draw(sides())); _total += _values[i];
		}
		_rolled = true;
	}
	/** @brief Browse results without rerolling, stopping at either end. @param step Direction. @return Page changed. */
	bool move_page(int8_t step) {
		if (!_rolled) { return false; }
		if (step > 0 && _page + 1 < pages()) { ++_page; return true; }
		if (step < 0 && _page) { --_page; return true; }
		return false;
	}
private:
	/** @brief Hide results from a previous pool without touching preferences. */
	void clear() { _rolled = false; _total = 0; _page = 0; }
	uint8_t _count = 1, _type = 5, _page = 0, _values[MAX_DICE] = {};
	uint16_t _total = 0;
	bool _rolled = false;
};

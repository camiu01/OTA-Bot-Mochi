/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/touch/swipe.h"
#include <assert.h>
#include <stdint.h>

int main() {
	SwipeTracker swipe;
	assert(swipe.update(true, 150, 100, 0) == 0);
	assert(swipe.update(true, 130, 100, 20) == 0);
	assert(swipe.update(true, 110, 100, 40) == 1);
	assert(swipe.update(true, 70, 100, 60) == 0);
	assert(swipe.update(false, 0, 0, 80) == 0);
	assert(swipe.update(true, 50, 100, 100) == 0);
	assert(swipe.update(true, 100, 110, 120) == -1);
	assert(swipe.update(false, 0, 0, 140) == 0);
	assert(swipe.update(true, 100, 100, 160) == 0);
	assert(swipe.update(true, 105, 160, 180) == 0);
	assert(swipe.update(false, 0, 0, 200) == 0);
	assert(swipe.update(true, 150, 100, 220) == 0);
	assert(swipe.update(true, 70, 100, 1221) == 0);
	assert(swipe.update(false, 0, 0, 1240) == 0);
	assert(swipe.update(true, 150, 100, UINT32_MAX - 10) == 0);
	assert(swipe.update(true, 100, 100, 20) == 1);
	return 0;
}

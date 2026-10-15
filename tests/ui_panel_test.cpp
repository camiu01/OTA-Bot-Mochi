/**
 * @file ui_panel_test.cpp
 * @brief Pages and menus on one panel geometry; the runner builds it per panel.
 *
 * The widget field stays the 240x280 design field (UI_DESIGN_W/UI_DESIGN_H),
 * so on any panel the same taps must reach the same widgets, every label must
 * fit the canvas (the raster mock aborts with a message otherwise), and the
 * empty margin of a larger panel must stay dead. Rendering runs on a native
 * mock; it does not prove touch digitizer calibration or panel wiring.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/screens.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/context_views.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <assert.h>
#include <iostream>
#include <string>

/** @brief Render every carousel and tool page once with realistic content. @param gfx Canvas. @param watch Chronos clock. */
static void page_render_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	Notification notification;
	notification.app = "Messages";
	notification.title = "A long title with multiple words for wrapping on this small display";
	notification.message = std::string(1400, 'x');
	ui_notification(notification);
	watch.navigation.active = true;
	watch.navigation.directions = "Turn right at the next street after the bridge";
	ui_config(watch, CF_NAV_DATA, 1, 0);
	watch.weather_count = 1;
	watch.weather[0].temp = 18;
	ui_config(watch, CF_WEATHER, 1, 0);
	// Prime a transition so the first loop entry also repaints from black.
	assert(screens_draw(&gfx, watch, UiPage::Clock, 0));
	for (uint8_t page = 0; page < uint8_t(UiPage::Count); ++page) {
		gfx.paints = 0;
		assert(screens_draw(&gfx, watch, static_cast<UiPage>(page), 1000 + page * 1000));
		assert(gfx.paints > 0);
		(void)screens_draw(&gfx, watch, static_cast<UiPage>(page), 1033 + page * 1000);
	}
}

/** @brief Tab cycling, row edits, and the settings footer stay reachable. */
static void menu_checks() {
	assert(settings_menu_tap(30, 70) == MenuAction::Changed);
	assert(settings_menu_tap(200, 70) == MenuAction::Changed);
	assert(settings_menu_tap(120, 70) == MenuAction::None);
	assert(settings_menu_tap(120, 93) == MenuAction::None);
	CompanionSettings before = settings_get();
	assert(settings_menu_tap(120, 110) == MenuAction::Changed);
	assert(settings_get().style != before.style);
	assert(settings_menu_tap(120, 160) == MenuAction::Changed);
	assert(settings_get().color != before.color);
	assert(settings_menu_tap(60, 250) == MenuAction::Changed);
	assert(!settings_dirty());
	assert(settings_menu_tap(180, 250) == MenuAction::Exit);
}

/** @brief Expression grid: cell hit, page scroll, then back to the tab rows. */
static void grid_checks() {
	CompanionSettings p = settings_get();
	p.mood = 0;
	settings_update(p);
	assert(settings_menu_tap(120, 200) == MenuAction::Changed);
	assert(settings_menu_grid_open());
	assert(settings_menu_tap(120, 107) == MenuAction::Changed);
	assert(settings_get().mood == 1);
	assert(settings_menu_scroll(1));
	assert(settings_menu_tap(120, 107) == MenuAction::Changed);
	assert(settings_get().mood == 4);
	assert(settings_menu_tap(180, 250) == MenuAction::Changed);
	assert(!settings_menu_grid_open());
	assert(settings_menu_scroll(1) || settings_menu_scroll(-1));
	assert(settings_menu_scroll(-1));
}

/** @brief Desk cards, timer buttons, and the margin outside the design field. */
static void desk_checks() {
	assert(screens_tap(UiPage::Desk, 20, 70, 0) == UiPage::BigClock);
	assert(screens_tap(UiPage::Desk, 200, 70, 0) == UiPage::Overview);
	assert(screens_tap(UiPage::Desk, 20, 170, 0) == UiPage::Timer);
	assert(screens_tap(UiPage::Desk, 200, 170, 0) == UiPage::Device);
	uint8_t mode = view_timer().mode;
	screens_tap(UiPage::Timer, 120, 180, 1000);
	assert(view_timer().running);
	screens_tap(UiPage::Timer, 20, 180, 1000);
	assert(!view_timer().running);
	screens_tap(UiPage::Timer, 20, 230, 1000);
	assert(view_timer().remaining == 1500000);
	screens_tap(UiPage::Timer, 150, 230, 1000);
	assert(view_timer().mode == uint8_t((mode + 1) % 3));
	// A larger panel must not let dead margin pixels press widgets that are
	// drawn only inside the 240x280 design field.
	if (LCD_WIDTH > UI_DESIGN_W) {
		assert(screens_tap(UiPage::Desk, UI_DESIGN_W + 10, 100, 0) == UiPage::Desk);
		uint8_t before = view_timer().mode;
		screens_tap(UiPage::Timer, UI_DESIGN_W + 20, 240, 0);
		assert(view_timer().mode == before);
		assert(settings_menu_tap(UI_DESIGN_W + 10, 110) == MenuAction::None);
	}
	if (LCD_HEIGHT > UI_DESIGN_H) {
		assert(screens_tap(UiPage::Desk, 20, UI_DESIGN_H + 10, 0) == UiPage::Desk);
		assert(settings_menu_tap(60, UI_DESIGN_H + 10) == MenuAction::None);
	}
}

/** @brief Run the layout proof for the panel the binary was compiled for. @return Success. */
int main() {
	assert(ui_state_init());
	assert(settings_init());
	ChronosESP32 watch;
	Arduino_GFX gfx;
	eyes_init(&gfx);
	settings_apply();
	page_render_checks(gfx, watch);
	menu_checks();
	grid_checks();
	desk_checks();
	std::cout << "Pages and menu layout passed on " << LCD_WIDTH << "x" << LCD_HEIGHT << "\n";
	return 0;
}

/**
 * @file ui_panel_test.cpp
 * @brief Pages and menus on one panel geometry; the runner builds it per panel.
 *
 * Every widget is laid out in the 240x280 design space and scaled to the
 * panel with ui_s()/ui_scale(), so on any panel the same design-space taps
 * must reach the same widgets, every label must fit the canvas (the raster
 * mock aborts with a message otherwise), and the strip below the scaled
 * design field must stay dead. Rendering runs on a native mock; it does not
 * prove touch digitizer calibration or panel wiring.
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
	assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
	assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
	assert(settings_menu_tap(ui_s(120), ui_s(70)) == MenuAction::None);
	assert(settings_menu_tap(ui_s(120), ui_s(93)) == MenuAction::None);
	CompanionSettings before = settings_get();
	assert(settings_menu_tap(ui_s(120), ui_s(110)) == MenuAction::Changed);
	assert(settings_get().style != before.style);
	assert(settings_menu_tap(ui_s(120), ui_s(160)) == MenuAction::Changed);
	assert(settings_get().color != before.color);
	assert(settings_menu_tap(ui_s(60), ui_s(250)) == MenuAction::Changed);
	assert(!settings_dirty());
	assert(settings_menu_tap(ui_s(180), ui_s(250)) == MenuAction::Exit);
}

/** @brief Expression grid: cell hit, page scroll, then back to the tab rows. */
static void grid_checks() {
	CompanionSettings p = settings_get();
	p.mood = 0;
	settings_update(p);
	assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
	assert(settings_menu_grid_open());
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 1);
	assert(settings_menu_scroll(1));
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 4);
	assert(settings_menu_tap(ui_s(180), ui_s(250)) == MenuAction::Changed);
	assert(!settings_menu_grid_open());
	assert(settings_menu_scroll(1) || settings_menu_scroll(-1));
	assert(settings_menu_scroll(-1));
}

/** @brief Desk cards, timer buttons, and the strip below the scaled field. */
static void desk_checks() {
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(70), 0) == UiPage::BigClock);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(70), 0) == UiPage::Overview);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(170), 0) == UiPage::Timer);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(170), 0) == UiPage::Device);
	uint8_t mode = view_timer().mode;
	screens_tap(UiPage::Timer, ui_s(120), ui_s(180), 1000);
	assert(view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(180), 1000);
	assert(!view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(230), 1000);
	assert(view_timer().remaining == 1500000);
	screens_tap(UiPage::Timer, ui_s(150), ui_s(230), 1000);
	assert(view_timer().mode == uint8_t((mode + 1) % 3));
	// A panel taller than the scaled design field grows black below it; that
	// strip must not press widgets drawn only inside the field.
	if (LCD_HEIGHT > ui_s(UI_DESIGN_H)) {
		assert(screens_tap(UiPage::Desk, LCD_WIDTH / 2, LCD_HEIGHT - 4, 0) == UiPage::Desk);
		assert(settings_menu_tap(LCD_WIDTH / 2, LCD_HEIGHT - 4) == MenuAction::None);
		uint8_t before = view_timer().mode;
		screens_tap(UiPage::Timer, LCD_WIDTH / 2, LCD_HEIGHT - 4, 0);
		assert(view_timer().mode == before);
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

#!/usr/bin/env bash
# Native regression checks only. This does not compile or upload firmware.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
output="$(mktemp -d)"
trap 'rm -rf "$output"' EXIT
runtime=(
	"$root/OTA-bot-Mochi/src/bus/i2c_bus.cpp"
	"$root/OTA-bot-Mochi/src/sound/es8311_output.cpp"
	"$root/OTA-bot-Mochi/src/settings/settings.cpp"
	"$root/OTA-bot-Mochi/src/settings/usb_commands.cpp"
	"$root/OTA-bot-Mochi/src/power/battery.cpp"
	"$root/OTA-bot-Mochi/src/power/charging.cpp"
	"$root/OTA-bot-Mochi/src/eyes/face_catalog.cpp"
	"$root/OTA-bot-Mochi/src/eyes/face_dispatch.cpp"
	"$root/OTA-bot-Mochi/src/eyes/face_primitives.cpp"
	"$root/OTA-bot-Mochi/src/eyes/face_glyph_primitives.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_shapes.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_lids.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_anim.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_expression.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_faces.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_life.cpp"
	"$root/OTA-bot-Mochi/src/ui/view_state.cpp"
	"$root/OTA-bot-Mochi/src/ui/ui_theme.cpp"
	"$root/OTA-bot-Mochi/src/ui/ui_state.cpp"
	"$root/OTA-bot-Mochi/src/ui/i18n.cpp"
	"$root/OTA-bot-Mochi/src/ui/weather_sync.cpp"
	"$root/OTA-bot-Mochi/src/sound/sound.cpp"
	"$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp"
	"$root/tests/stubs/eyes_draw.cpp"
	"$root/tests/stubs/backlight.cpp"
)
# Build and run one native test, stopping on failure.
build_test() {
	local name="$1"
	shift
	local board=()
	local network=()
	local companion=()
	local source
	if [[ " $* " == *"/src/update/ota_update.cpp"* ]]; then
		network=("$root/OTA-bot-Mochi/src/network/network_config.cpp"
			"$root/OTA-bot-Mochi/src/network/network_service.cpp" "$root/tests/stubs/network_weather.cpp")
	fi
	if [[ " $* " == *"/src/ui/screens.cpp"* ]]; then
		for source in companion_state companion_service companion_language phone_view phone_qr; do
			companion+=("$root/OTA-bot-Mochi/src/companion/$source.cpp")
		done
	fi
	if [[ " $* " != *" -DMOCHI_BOARD_AMOLED_18 "* ]]; then board=(-DMOCHI_BOARD_LCD_169); fi
	if [[ ${#companion[@]} -gt 0 || " $* " == *"/src/companion/companion_service.cpp"* ]]; then
		companion+=("$root/tests/stubs/backlight_activity.cpp")
	fi
	g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" "${board[@]}" "$@" "${network[@]}" "${companion[@]}" -o "$output/$name"
	"$output/$name"
}
python3 "$root/tests/board_profiles_test.py"
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" "$root/tools/export-sounds.cpp" -o "$output/export-sounds"
python3 "$root/tests/sound_export_test.py" "$output/export-sounds"
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" -DMOCHI_BOARD_LCD_169 -fsyntax-only "$root/OTA-bot-Mochi/src/touch/touch.cpp"
# The second board profile must also parse: touch reset, backlight pin, battery
# ADC, and buzzer branches all switch with -DMOCHI_BOARD_AMOLED_18.
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" -DMOCHI_BOARD_AMOLED_18 -fsyntax-only \
	"$root/OTA-bot-Mochi/src/touch/touch.cpp" "$root/OTA-bot-Mochi/src/power/backlight.cpp" \
	"$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp" "$root/OTA-bot-Mochi/src/power/battery.cpp"
build_test swipe "$root/tests/swipe_test.cpp"
build_test ancs "$root/tests/ancs_test.cpp" "$root/OTA-bot-Mochi/src/ancs/ancs_parser.cpp"
build_test utf8-text "$root/tests/utf8_text_test.cpp" "$root/OTA-bot-Mochi/src/ui/ui_state.cpp"
boot=("$root/tests/ota_boot_test.cpp" "$root/OTA-bot-Mochi/src/update/ota_boot.cpp")
build_test ota-boot -I "$root/tests/stubs/ota" -DCONFIG_APP_ROLLBACK_ENABLE=1 -DCONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=1 "${boot[@]}"
build_test ota-boot-disabled -I "$root/tests/stubs/ota" "${boot[@]}"
build_test ams -I "$root/tests/stubs/media" "$root/tests/ams_test.cpp" "$root/OTA-bot-Mochi/src/ancs/ams_service.cpp" "$root/OTA-bot-Mochi/src/ui/ui_state.cpp"
build_test sd-audio -DMOCHI_BOARD_AMOLED_18 "$root/tests/sd_audio_test.cpp"
sd_update=("$root/tests/sd_update_test.cpp" "$root/OTA-bot-Mochi/src/update/sd_update.cpp")
build_test sd-update-lcd -I "$root/tests/stubs/ota" "${sd_update[@]}"
build_test sd-update-amoled -I "$root/tests/stubs/ota" -DMOCHI_BOARD_AMOLED_18 "${sd_update[@]}"
build_test music "$root/tests/music_test.cpp" "$root/OTA-bot-Mochi/src/ui/ui_state.cpp"
charging=("$root/tests/charging_test.cpp" "$root/OTA-bot-Mochi/src/power/charging.cpp" "$root/OTA-bot-Mochi/src/bus/i2c_bus.cpp")
build_test charging-lcd "${charging[@]}"
build_test charging-amoled -DMOCHI_BOARD_AMOLED_18 "${charging[@]}"
battery=("$root/tests/battery_test.cpp" "$root/OTA-bot-Mochi/src/power/battery.cpp" "$root/OTA-bot-Mochi/src/power/charging.cpp" "$root/OTA-bot-Mochi/src/bus/i2c_bus.cpp")
build_test battery-lcd "${battery[@]}"
build_test battery-amoled -DMOCHI_BOARD_AMOLED_18 "${battery[@]}"
build_test battery-chronos "$root/tests/battery_chronos_test.cpp"
build_test tic-tac-toe "$root/tests/tic_tac_toe_test.cpp"
build_test dice "$root/tests/dice_test.cpp"
ota=("${runtime[@]}" "$root/tests/ota_test.cpp" "$root/OTA-bot-Mochi/src/update/ota_update.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test ota-lcd -I "$root/tests/stubs/ota" "${ota[@]}"
build_test ota-amoled -I "$root/tests/stubs/ota" -DMOCHI_BOARD_AMOLED_18 "${ota[@]}"
ota_qr=("$root/tests/ota_qr_test.cpp" "$root/OTA-bot-Mochi/src/update/ota_qr.cpp")
build_test ota-qr-lcd -I "$root/tests/stubs/ota" "${ota_qr[@]}"
build_test ota-qr-amoled -I "$root/tests/stubs/ota" -DMOCHI_BOARD_AMOLED_18 "${ota_qr[@]}"
ota_screen=("$root/tests/ota_screen_test.cpp" "$root/OTA-bot-Mochi/src/update/ota_screen.cpp"
	"$root/OTA-bot-Mochi/src/update/ota_qr.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test ota-screen-lcd -I "$root/tests/stubs/ota" "${runtime[@]}" "${ota_screen[@]}"
build_test ota-screen-amoled -I "$root/tests/stubs/ota" -DMOCHI_BOARD_AMOLED_18 "${runtime[@]}" "${ota_screen[@]}"
sd_screen=("$root/tests/sd_update_screen_test.cpp" "$root/OTA-bot-Mochi/src/update/sd_update_screen.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test sd-update-screen-lcd "${runtime[@]}" "${sd_screen[@]}"
build_test sd-update-screen-amoled -DMOCHI_BOARD_AMOLED_18 "${runtime[@]}" "${sd_screen[@]}"
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" -DMOCHI_BOARD_LCD_169 -fsyntax-only "$root/OTA-bot-Mochi/src/update/ota_screen.cpp"
build_test startup "$root/tests/startup_test.cpp"
build_test face-variety "$root/tests/face_variety_test.cpp"
build_test render-metrics "$root/tests/render_metrics_test.cpp"
build_test device-status -DARDUINO_ARCH_ESP32 -I "$root/tests/stubs/diagnostics" -I "$root/tests/stubs/ota" "$root/tests/device_status_test.cpp"
power_button=("$root/tests/power_button_test.cpp" "$root/OTA-bot-Mochi/src/power/power_button.cpp" "$root/OTA-bot-Mochi/src/power/power_button_lcd.cpp" "$root/OTA-bot-Mochi/src/bus/i2c_bus.cpp")
build_test power-button-lcd "${power_button[@]}"
build_test power-button-amoled -DMOCHI_BOARD_AMOLED_18 "${power_button[@]}"
# This legacy fixture uses 240x280 touch coordinates; ui-panel covers scaled panels.
build_test ui -DLCD_WIDTH=240 -DLCD_HEIGHT=280 "${runtime[@]}" \
	"$root/tests/ui_test.cpp" \
	"$root/OTA-bot-Mochi/src/ui/screens.cpp" \
	"$root/OTA-bot-Mochi/src/ui/context_views.cpp" \
	"$root/OTA-bot-Mochi/src/clock/clock_ui.cpp" \
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test imu "${runtime[@]}" \
	"$root/tests/imu_test.cpp" \
	"$root/OTA-bot-Mochi/src/eyes/eyes_imu.cpp"
build_test settings "${runtime[@]}" \
	"$root/tests/settings_test.cpp" \
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp" \
	"$root/tests/stubs/imu_noop.cpp"
render_runtime=()
build_test night-schedule "${runtime[@]}" "$root/tests/night_schedule_test.cpp" \
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test face-preset "${runtime[@]}" "$root/tests/face_preset_test.cpp" "$root/tests/stubs/imu_noop.cpp"
for source in "${runtime[@]}"; do
	if [[ "$source" != "$root/tests/stubs/eyes_draw.cpp" ]]; then render_runtime+=("$source"); fi
done
render_eyes=("$root/OTA-bot-Mochi/src/eyes/eyes_draw.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test eyes "${render_runtime[@]}" "$root/tests/eyes_test.cpp" "${render_eyes[@]}"
build_test face-variation -O2 "${render_runtime[@]}" "$root/tests/face_variation_test.cpp" "${render_eyes[@]}"
build_test face-filter "${render_runtime[@]}" "$root/tests/face_filter_test.cpp" \
	"$root/OTA-bot-Mochi/src/eyes/eyes_draw.cpp" "$root/OTA-bot-Mochi/src/eyes/eyes_imu.cpp"
build_test face-web -I "$root/tests/stubs/ota" "${runtime[@]}" "$root/tests/face_web_test.cpp" \
	"$root/OTA-bot-Mochi/src/update/ota_update.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test settings-web -I "$root/tests/stubs/ota" "${runtime[@]}" "$root/tests/settings_web_test.cpp" \
	"$root/OTA-bot-Mochi/src/update/ota_update.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test settings-web-amoled -DMOCHI_BOARD_AMOLED_18 -I "$root/tests/stubs/ota" "${runtime[@]}" "$root/tests/settings_web_test.cpp" \
	"$root/OTA-bot-Mochi/src/update/ota_update.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test ntp-sync -I "$root/tests/stubs/ota" "$root/tests/ntp_sync_test.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp"
build_test network -I "$root/tests/stubs/ota" "${runtime[@]}" "$root/tests/network_test.cpp" \
	"$root/OTA-bot-Mochi/src/update/ota_update.cpp" "$root/OTA-bot-Mochi/src/update/ntp_sync.cpp" "$root/tests/stubs/imu_noop.cpp"
build_test network-weather -I "$root/tests/stubs/ota" -I "$root/tests/stubs/diagnostics" "$root/tests/network_weather_test.cpp" "$root/OTA-bot-Mochi/src/network/network_weather_parse.cpp"
build_test face-preset-raster "${render_runtime[@]}" "$root/tests/face_preset_raster_test.cpp" "${render_eyes[@]}"
build_test all-faces-raster -O2 -DALL_FACES_RASTER "${render_runtime[@]}" "$root/tests/face_preset_raster_test.cpp" "${render_eyes[@]}"
build_test faces "${render_runtime[@]}" "$root/tests/face_unique_test.cpp" "${render_eyes[@]}"
build_test panel "${render_runtime[@]}" "$root/tests/eye_scaling_test.cpp" "${render_eyes[@]}"
build_test accent "${render_runtime[@]}" "$root/tests/eye_accent_test.cpp" "${render_eyes[@]}"
build_test reference "${render_runtime[@]}" "$root/tests/reference_test.cpp" "${render_eyes[@]}"
build_test performance "${render_runtime[@]}" "$root/tests/performance_test.cpp" "${render_eyes[@]}"
build_test samurai "${render_runtime[@]}" "$root/tests/samurai_test.cpp" "${render_eyes[@]}"
build_test uwu "${render_runtime[@]}" "$root/tests/uwu_test.cpp" "${render_eyes[@]}"
build_test neon "${render_runtime[@]}" "$root/tests/neon_test.cpp" "${render_eyes[@]}"
build_test face-background "${render_runtime[@]}" "$root/tests/face_background_test.cpp" "${render_eyes[@]}"
ui_panel=("${runtime[@]}" \
	"$root/tests/ui_panel_test.cpp" \
	"$root/OTA-bot-Mochi/src/ui/screens.cpp" \
	"$root/OTA-bot-Mochi/src/ui/context_views.cpp" \
	"$root/OTA-bot-Mochi/src/clock/clock_ui.cpp" \
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test ui-panel "${ui_panel[@]}"
build_test companion "${runtime[@]}" "$root/tests/companion_test.cpp" "$root/tests/stubs/imu_noop.cpp" \
	"$root/OTA-bot-Mochi/src/companion/companion_state.cpp" "$root/OTA-bot-Mochi/src/companion/companion_service.cpp" \
	"$root/OTA-bot-Mochi/src/companion/companion_language.cpp"
phone_panel=()
for source in "${ui_panel[@]}"; do
	if [[ "$source" != "$root/tests/ui_panel_test.cpp" ]]; then phone_panel+=("$source"); fi
done
phone_panel+=("$root/tests/companion_panel_test.cpp")
build_test companion-panel-lcd "${phone_panel[@]}"
build_test companion-panel-amoled -DMOCHI_BOARD_AMOLED_18 "${phone_panel[@]}"
build_test ui-panel-240x320 -DLCD_WIDTH=240 -DLCD_HEIGHT=320 "${ui_panel[@]}"
build_test ui-panel-368x448 -DMOCHI_BOARD_AMOLED_18 "${ui_panel[@]}"
build_test sound "${runtime[@]}" "$root/tests/sound_test.cpp" "$root/tests/stubs/imu_noop.cpp"
pwm=("$root/tests/pwm_test.cpp" "$root/OTA-bot-Mochi/src/power/backlight.cpp" "$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp")
build_test pwm-v3 "${pwm[@]}"
build_test pwm-v2 -DESP_ARDUINO_VERSION_MAJOR=2 "${pwm[@]}"
build_test es8311 -DMOCHI_BOARD_AMOLED_18 "$root/tests/es8311_test.cpp" \
	"$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp" \
	"$root/OTA-bot-Mochi/src/sound/es8311_output.cpp" "$root/OTA-bot-Mochi/src/sound/sound.cpp" \
	"$root/OTA-bot-Mochi/src/bus/i2c_bus.cpp"
printf 'All host tests passed\n'

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
	if [[ " $* " != *" -DMOCHI_BOARD_AMOLED_18 "* ]]; then board=(-DMOCHI_BOARD_LCD_169); fi
	g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" "${board[@]}" "$@" -o "$output/$name"
	"$output/$name"
}
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" -DMOCHI_BOARD_LCD_169 -fsyntax-only "$root/OTA-bot-Mochi/src/touch/touch.cpp"
# The second board profile must also parse: touch reset, backlight pin, battery
# ADC, and buzzer branches all switch with -DMOCHI_BOARD_AMOLED_18.
g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" -DMOCHI_BOARD_AMOLED_18 -fsyntax-only \
	"$root/OTA-bot-Mochi/src/touch/touch.cpp" "$root/OTA-bot-Mochi/src/power/backlight.cpp" \
	"$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp" "$root/OTA-bot-Mochi/src/power/battery.cpp"
build_test swipe "$root/tests/swipe_test.cpp"
build_test startup "$root/tests/startup_test.cpp"
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
for source in "${runtime[@]}"; do
	if [[ "$source" != "$root/tests/stubs/eyes_draw.cpp" ]]; then render_runtime+=("$source"); fi
done
render_eyes=("$root/OTA-bot-Mochi/src/eyes/eyes_draw.cpp" "$root/OTA-bot-Mochi/src/eyes/eye_shapes.cpp" \
	"$root/OTA-bot-Mochi/src/eyes/eye_lids.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test eyes "${render_runtime[@]}" "$root/tests/eyes_test.cpp" "${render_eyes[@]}"
build_test faces "${render_runtime[@]}" "$root/tests/face_unique_test.cpp" "${render_eyes[@]}"
build_test panel "${render_runtime[@]}" "$root/tests/eye_scaling_test.cpp" "${render_eyes[@]}"
build_test accent "${render_runtime[@]}" "$root/tests/eye_accent_test.cpp" "${render_eyes[@]}"
build_test reference "${render_runtime[@]}" "$root/tests/reference_test.cpp" "${render_eyes[@]}"
build_test performance "${render_runtime[@]}" "$root/tests/performance_test.cpp" "${render_eyes[@]}"
ui_panel=("${runtime[@]}" \
	"$root/tests/ui_panel_test.cpp" \
	"$root/OTA-bot-Mochi/src/ui/screens.cpp" \
	"$root/OTA-bot-Mochi/src/ui/context_views.cpp" \
	"$root/OTA-bot-Mochi/src/clock/clock_ui.cpp" \
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp" "$root/tests/stubs/imu_noop.cpp")
build_test ui-panel "${ui_panel[@]}"
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

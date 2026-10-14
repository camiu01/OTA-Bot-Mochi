#!/usr/bin/env bash
# Build and run the host SDL2 LCD emulator. Native preview only; no board needed.
# Requires: g++ (C++17), pkg-config, and the SDL2 development package.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
output="${TMPDIR:-/tmp}/mochi-emulator"
mkdir -p "$output"

if ! pkg-config --exists sdl2; then
	echo "SDL2 development files not found (install libsdl2-dev / SDL2-devel)" >&2
	exit 1
fi

# Same sources as scripts/test-host.sh, but with the real renderer and backlight
# instead of their test doubles, plus the SDL front end.
runtime=(
	"$root/OTA-bot-Mochi/src/settings/settings.cpp"
	"$root/OTA-bot-Mochi/src/settings/usb_commands.cpp"
	"$root/OTA-bot-Mochi/src/settings/settings_menu.cpp"
	"$root/OTA-bot-Mochi/src/power/battery.cpp"
	"$root/OTA-bot-Mochi/src/power/backlight.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_anim.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_expression.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_faces.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_life.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eyes_draw.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_shapes.cpp"
	"$root/OTA-bot-Mochi/src/eyes/eye_lids.cpp"
	"$root/OTA-bot-Mochi/src/ui/view_state.cpp"
	"$root/OTA-bot-Mochi/src/ui/ui_theme.cpp"
	"$root/OTA-bot-Mochi/src/ui/ui_state.cpp"
	"$root/OTA-bot-Mochi/src/ui/i18n.cpp"
	"$root/OTA-bot-Mochi/src/ui/weather_sync.cpp"
	"$root/OTA-bot-Mochi/src/ui/screens.cpp"
	"$root/OTA-bot-Mochi/src/ui/context_views.cpp"
	"$root/OTA-bot-Mochi/src/clock/clock_ui.cpp"
	"$root/OTA-bot-Mochi/src/sound/sound.cpp"
	"$root/OTA-bot-Mochi/src/sound/buzzer_output.cpp"
	"$root/tests/stubs/imu_noop.cpp"
	"$root/tools/lcd_emulator.cpp"
	"$root/tools/emulator_input.cpp"
)

# shellcheck disable=SC2046
binary="$output/lcd-emulator"
inputs=("${runtime[@]}" "$root/OTA-bot-Mochi/pin_config.h" \
	$(find "$root/OTA-bot-Mochi/src" "$root/tests/stubs" "$root/tools" -name '*.h'))
rebuild=1
if [[ -x "$binary" ]]; then
	rebuild=0
	for src in "${inputs[@]}"; do
		if [[ "$src" -nt "$binary" ]]; then rebuild=1; break; fi
	done
fi

if [[ "$rebuild" == 1 ]]; then
	g++ -std=c++17 -Wall -Wextra -Werror -I "$root/tests/stubs" \
		$(pkg-config --cflags sdl2) "${runtime[@]}" \
		-o "$binary" $(pkg-config --libs sdl2)
fi

exec "$binary" "$@"

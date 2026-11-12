# Native regression checks only. This does not compile or upload firmware.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$output = Join-Path ([System.IO.Path]::GetTempPath()) ("mochi-tests-" + [guid]::NewGuid())
New-Item -ItemType Directory -Path $output | Out-Null
# Build one native executable and stop on the first failure.
function Invoke-HostTest([string] $Name, [string[]] $Sources, [string[]] $Flags = @()) {
	if ($Sources -contains "$root\OTA-bot-Mochi\src\update\ota_update.cpp") { $Sources += $network }
	if ($Sources -contains "$root\OTA-bot-Mochi\src\ui\screens.cpp") { $Sources += $companion }
	if ($Sources -contains "$root\OTA-bot-Mochi\src\companion\companion_service.cpp") { $Sources += "$root\tests\stubs\backlight_activity.cpp" }
	# An IDE-local AMOLED selection must not change the LCD baseline fixtures.
	if ($Flags -notcontains '-DMOCHI_BOARD_AMOLED_18') { $Flags += '-DMOCHI_BOARD_LCD_169' }
	& g++ -std=c++17 -Wall -Wextra -Werror @Flags -I "$root\tests\stubs" @Sources -o "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name test build failed" }
	& "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name tests failed" }
}
$network = @("$root\OTA-bot-Mochi\src\network\network_config.cpp",
	"$root\OTA-bot-Mochi\src\network\network_service.cpp", "$root\tests\stubs\network_weather.cpp")
$companion = @('companion_state.cpp', 'companion_service.cpp', 'companion_language.cpp', 'phone_view.cpp', 'phone_qr.cpp') |
	ForEach-Object { Join-Path $root "OTA-bot-Mochi\src\companion\$_" }
try {
	& python "$root\tests\board_profiles_test.py"
	if ($LASTEXITCODE -ne 0) { throw 'Board profile checks failed' }
	& g++ -std=c++17 -Wall -Wextra -Werror -I "$root\tests\stubs" "$root\tools\export-sounds.cpp" -o "$output\export-sounds.exe"
	if ($LASTEXITCODE -ne 0) { throw 'Sound WAV exporter build failed' }
	& python "$root\tests\sound_export_test.py" "$output\export-sounds.exe"
	if ($LASTEXITCODE -ne 0) { throw 'Sound WAV exporter tests failed' }
	& g++ -std=c++17 -Wall -Wextra -Werror -I "$root\tests\stubs" -DMOCHI_BOARD_LCD_169 -fsyntax-only "$root\OTA-bot-Mochi\src\touch\touch.cpp"
	if ($LASTEXITCODE -ne 0) { throw 'Touch driver syntax check failed' }
	# The second board profile must also parse: touch reset, backlight pin,
	# battery ADC, and buzzer branches all switch with -DMOCHI_BOARD_AMOLED_18.
	& g++ -std=c++17 -Wall -Wextra -Werror -I "$root\tests\stubs" -DMOCHI_BOARD_AMOLED_18 -fsyntax-only `
		"$root\OTA-bot-Mochi\src\touch\touch.cpp" "$root\OTA-bot-Mochi\src\power\backlight.cpp" `
		"$root\OTA-bot-Mochi\src\sound\buzzer_output.cpp" "$root\OTA-bot-Mochi\src\power\battery.cpp"
	if ($LASTEXITCODE -ne 0) { throw 'AMOLED profile syntax check failed' }
	$runtime = @(
		"$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp",
		"$root\OTA-bot-Mochi\src\sound\es8311_output.cpp",
		"$root\OTA-bot-Mochi\src\settings\settings.cpp",
		"$root\OTA-bot-Mochi\src\settings\usb_commands.cpp",
		"$root\OTA-bot-Mochi\src\power\battery.cpp",
		"$root\OTA-bot-Mochi\src\power\charging.cpp",
		"$root\OTA-bot-Mochi\src\eyes\face_catalog.cpp",
		"$root\OTA-bot-Mochi\src\eyes\face_dispatch.cpp",
		"$root\OTA-bot-Mochi\src\eyes\face_primitives.cpp",
		"$root\OTA-bot-Mochi\src\eyes\face_glyph_primitives.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eye_shapes.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eye_lids.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eyes_anim.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eyes_expression.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eyes_faces.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eye_life.cpp",
		"$root\OTA-bot-Mochi\src\ui\view_state.cpp",
		"$root\OTA-bot-Mochi\src\ui\ui_theme.cpp",
		"$root\OTA-bot-Mochi\src\ui\ui_state.cpp",
		"$root\OTA-bot-Mochi\src\ui\i18n.cpp",
		"$root\OTA-bot-Mochi\src\ui\weather_sync.cpp",
		"$root\OTA-bot-Mochi\src\sound\sound.cpp",
		"$root\OTA-bot-Mochi\src\sound\buzzer_output.cpp",
		"$root\tests\stubs\eyes_draw.cpp",
		"$root\tests\stubs\backlight.cpp"
	)
	$ui = @(
		"$root\tests\ui_test.cpp",
		"$root\OTA-bot-Mochi\src\ui\screens.cpp",
		"$root\OTA-bot-Mochi\src\ui\context_views.cpp",
		"$root\OTA-bot-Mochi\src\clock\clock_ui.cpp",
		"$root\OTA-bot-Mochi\src\settings\settings_menu.cpp",
		"$root\tests\stubs\imu_noop.cpp"
	)
	Invoke-HostTest 'swipe' @("$root\tests\swipe_test.cpp")
	Invoke-HostTest 'pickup' @("$root\tests\pickup_test.cpp")
	$touch = @("$root\tests\touch_contact_test.cpp", "$root\OTA-bot-Mochi\src\touch\touch.cpp", "$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp")
	Invoke-HostTest 'touch-contact-lcd' $touch
	Invoke-HostTest 'touch-contact-amoled' $touch @('-DMOCHI_BOARD_AMOLED_18')
	Invoke-HostTest 'ancs' @("$root\tests\ancs_test.cpp", "$root\OTA-bot-Mochi\src\ancs\ancs_parser.cpp")
	Invoke-HostTest 'utf8-text' @("$root\tests\utf8_text_test.cpp", "$root\OTA-bot-Mochi\src\ui\ui_state.cpp")
	$boot = @("$root\tests\ota_boot_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_boot.cpp")
	Invoke-HostTest 'ota-boot' $boot @('-I', "$root\tests\stubs\ota", '-DCONFIG_APP_ROLLBACK_ENABLE=1', '-DCONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=1')
	Invoke-HostTest 'ota-boot-disabled' $boot @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'ams' @("$root\tests\ams_test.cpp", "$root\OTA-bot-Mochi\src\ancs\ams_service.cpp", "$root\OTA-bot-Mochi\src\ui\ui_state.cpp") @('-I', "$root\tests\stubs\media")
	Invoke-HostTest 'sd-audio' @("$root\tests\sd_audio_test.cpp") @('-DMOCHI_BOARD_AMOLED_18')
	$sdUpdate = @("$root\tests\sd_update_test.cpp", "$root\OTA-bot-Mochi\src\update\sd_update.cpp")
	Invoke-HostTest 'sd-update-lcd' $sdUpdate @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'sd-update-amoled' $sdUpdate @('-I', "$root\tests\stubs\ota", '-DMOCHI_BOARD_AMOLED_18')
	Invoke-HostTest 'music' @("$root\tests\music_test.cpp", "$root\OTA-bot-Mochi\src\ui\ui_state.cpp")
	$charging = @("$root\tests\charging_test.cpp", "$root\OTA-bot-Mochi\src\power\charging.cpp", "$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp")
	Invoke-HostTest 'charging-lcd' $charging
	Invoke-HostTest 'charging-amoled' $charging @('-DMOCHI_BOARD_AMOLED_18')
	$battery = @("$root\tests\battery_test.cpp", "$root\OTA-bot-Mochi\src\power\battery.cpp", "$root\OTA-bot-Mochi\src\power\charging.cpp", "$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp")
	Invoke-HostTest 'battery-lcd' $battery
	Invoke-HostTest 'battery-amoled' $battery @('-DMOCHI_BOARD_AMOLED_18')
	Invoke-HostTest 'battery-chronos' @("$root\tests\battery_chronos_test.cpp")
	Invoke-HostTest 'tic-tac-toe' @("$root\tests\tic_tac_toe_test.cpp")
	Invoke-HostTest 'dice' @("$root\tests\dice_test.cpp")
	Invoke-HostTest 'dice-geometry' @("$root\tests\dice_geometry_test.cpp")
	Invoke-HostTest 'flappy' @("$root\tests\flappy_test.cpp")
	$ota = $runtime + @("$root\tests\ota_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_update.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp", "$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'ota-lcd' $ota @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'ota-amoled' $ota @('-I', "$root\tests\stubs\ota", '-DMOCHI_BOARD_AMOLED_18')
	$otaQr = @("$root\tests\ota_qr_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_qr.cpp")
	Invoke-HostTest 'ota-qr-lcd' $otaQr @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'ota-qr-amoled' $otaQr @('-I', "$root\tests\stubs\ota", '-DMOCHI_BOARD_AMOLED_18')
	$otaScreen = @("$root\tests\ota_screen_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_screen.cpp",
		"$root\OTA-bot-Mochi\src\update\ota_qr.cpp", "$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'ota-screen-lcd' ($runtime + $otaScreen) @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'ota-screen-amoled' ($runtime + $otaScreen) @('-I', "$root\tests\stubs\ota", '-DMOCHI_BOARD_AMOLED_18')
	$sdScreen = @("$root\tests\sd_update_screen_test.cpp", "$root\OTA-bot-Mochi\src\update\sd_update_screen.cpp", "$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'sd-update-screen-lcd' ($runtime + $sdScreen)
	Invoke-HostTest 'sd-update-screen-amoled' ($runtime + $sdScreen) @('-DMOCHI_BOARD_AMOLED_18')
	& g++ -std=c++17 -Wall -Wextra -Werror -I "$root\tests\stubs" -DMOCHI_BOARD_LCD_169 -fsyntax-only "$root\OTA-bot-Mochi\src\update\ota_screen.cpp"
	if ($LASTEXITCODE -ne 0) { throw 'OTA screen syntax check failed' }
	Invoke-HostTest 'startup' @("$root\tests\startup_test.cpp")
	Invoke-HostTest 'face-variety' @("$root\tests\face_variety_test.cpp")
	Invoke-HostTest 'render-metrics' @("$root\tests\render_metrics_test.cpp")
	Invoke-HostTest 'device-status' @("$root\tests\device_status_test.cpp") @('-DARDUINO_ARCH_ESP32', '-I', "$root\tests\stubs\diagnostics", '-I', "$root\tests\stubs\ota")
	$powerButton = @("$root\tests\power_button_test.cpp", "$root\OTA-bot-Mochi\src\power\power_button.cpp",
		"$root\OTA-bot-Mochi\src\power\power_button_lcd.cpp", "$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp")
	Invoke-HostTest 'power-button-lcd' $powerButton
	Invoke-HostTest 'power-button-amoled' $powerButton @('-DMOCHI_BOARD_AMOLED_18')
	# This legacy fixture uses 240x280 touch coordinates; ui-panel covers scaled panels.
	Invoke-HostTest 'ui' ($runtime + $ui) @('-DLCD_WIDTH=240', '-DLCD_HEIGHT=280')
	Invoke-HostTest 'imu' ($runtime + @("$root\tests\imu_test.cpp", "$root\OTA-bot-Mochi\src\eyes\eyes_imu.cpp"))
	$settings = @("$root\tests\settings_test.cpp", "$root\OTA-bot-Mochi\src\settings\settings_menu.cpp",
		"$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'settings' ($runtime + $settings)
	Invoke-HostTest 'night-schedule' ($runtime + @("$root\tests\night_schedule_test.cpp", "$root\OTA-bot-Mochi\src\settings\settings_menu.cpp", "$root\tests\stubs\imu_noop.cpp"))
	Invoke-HostTest 'face-preset' ($runtime + @("$root\tests\face_preset_test.cpp", "$root\tests\stubs\imu_noop.cpp"))
	$renderRuntime = @($runtime | Where-Object { $_ -ne "$root\tests\stubs\eyes_draw.cpp" })
	$renderEyes = @("$root\OTA-bot-Mochi\src\eyes\eyes_draw.cpp", "$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'eyes' ($renderRuntime + @("$root\tests\eyes_test.cpp") + $renderEyes)
	Invoke-HostTest 'animated-emotions' ($renderRuntime + @("$root\tests\animated_emotions_test.cpp") + $renderEyes) @('-O2')
	Invoke-HostTest 'face-variation' ($renderRuntime + @("$root\tests\face_variation_test.cpp") + $renderEyes) @('-O2')
	Invoke-HostTest 'face-interaction' ($renderRuntime + @("$root\tests\face_interaction_test.cpp") + $renderEyes) @('-O2')
	Invoke-HostTest 'face-filter' ($renderRuntime + @("$root\tests\face_filter_test.cpp", "$root\OTA-bot-Mochi\src\eyes\eyes_draw.cpp", "$root\OTA-bot-Mochi\src\eyes\eyes_imu.cpp"))
	Invoke-HostTest 'face-web' ($runtime + @("$root\tests\face_web_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_update.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp", "$root\tests\stubs\imu_noop.cpp")) @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'settings-web' ($runtime + @("$root\tests\settings_web_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_update.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp", "$root\tests\stubs\imu_noop.cpp")) @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'settings-web-amoled' ($runtime + @("$root\tests\settings_web_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_update.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp", "$root\tests\stubs\imu_noop.cpp")) @('-DMOCHI_BOARD_AMOLED_18', '-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'network' ($runtime + @("$root\tests\network_test.cpp", "$root\OTA-bot-Mochi\src\update\ota_update.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp", "$root\tests\stubs\imu_noop.cpp")) @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'network-weather' @("$root\tests\network_weather_test.cpp", "$root\OTA-bot-Mochi\src\network\network_weather_parse.cpp") @('-I', "$root\tests\stubs\ota", '-I', "$root\tests\stubs\diagnostics")
	Invoke-HostTest 'ntp-sync' @("$root\tests\ntp_sync_test.cpp", "$root\OTA-bot-Mochi\src\update\ntp_sync.cpp") @('-I', "$root\tests\stubs\ota")
	Invoke-HostTest 'face-preset-raster' ($renderRuntime + @("$root\tests\face_preset_raster_test.cpp") + $renderEyes)
	Invoke-HostTest 'all-faces-raster' ($renderRuntime + @("$root\tests\face_preset_raster_test.cpp") + $renderEyes) @('-O2', '-DALL_FACES_RASTER')
	Invoke-HostTest 'faces' ($renderRuntime + @("$root\tests\face_unique_test.cpp") + $renderEyes)
	Invoke-HostTest 'panel' ($renderRuntime + @("$root\tests\eye_scaling_test.cpp") + $renderEyes)
	Invoke-HostTest 'accent' ($renderRuntime + @("$root\tests\eye_accent_test.cpp") + $renderEyes)
	Invoke-HostTest 'reference' ($renderRuntime + @("$root\tests\reference_test.cpp") + $renderEyes)
	Invoke-HostTest 'performance' ($renderRuntime + @("$root\tests\performance_test.cpp") + $renderEyes)
	Invoke-HostTest 'samurai' ($renderRuntime + @("$root\tests\samurai_test.cpp") + $renderEyes)
	Invoke-HostTest 'uwu' ($renderRuntime + @("$root\tests\uwu_test.cpp") + $renderEyes)
	Invoke-HostTest 'neon' ($renderRuntime + @("$root\tests\neon_test.cpp") + $renderEyes)
	Invoke-HostTest 'face-background' ($renderRuntime + @("$root\tests\face_background_test.cpp") + $renderEyes)
	$uiPanel = @(
		"$root\tests\ui_panel_test.cpp",
		"$root\OTA-bot-Mochi\src\ui\screens.cpp",
		"$root\OTA-bot-Mochi\src\ui\context_views.cpp",
		"$root\OTA-bot-Mochi\src\clock\clock_ui.cpp",
		"$root\OTA-bot-Mochi\src\settings\settings_menu.cpp",
		"$root\tests\stubs\imu_noop.cpp"
	)
	$companionService = @("$root\tests\companion_test.cpp", "$root\tests\stubs\imu_noop.cpp") + $companion[0..2]
	Invoke-HostTest 'companion' ($runtime + $companionService)
	$phonePanel = @($uiPanel | Where-Object { $_ -ne "$root\tests\ui_panel_test.cpp" }) + "$root\tests\companion_panel_test.cpp"
	Invoke-HostTest 'companion-panel-lcd' ($runtime + $phonePanel)
	Invoke-HostTest 'companion-panel-amoled' ($runtime + $phonePanel) @('-DMOCHI_BOARD_AMOLED_18')
	Invoke-HostTest 'ui-panel' ($runtime + $uiPanel)
	Invoke-HostTest 'ui-panel-240x320' ($runtime + $uiPanel) @('-DLCD_WIDTH=240', '-DLCD_HEIGHT=320')
	Invoke-HostTest 'ui-panel-368x448' ($runtime + $uiPanel) @('-DMOCHI_BOARD_AMOLED_18')
	Invoke-HostTest 'sound' ($runtime + @("$root\tests\sound_test.cpp", "$root\tests\stubs\imu_noop.cpp"))
	$pwm = @("$root\tests\pwm_test.cpp", "$root\OTA-bot-Mochi\src\power\backlight.cpp", "$root\OTA-bot-Mochi\src\sound\buzzer_output.cpp")
	Invoke-HostTest 'pwm-v3' $pwm
	Invoke-HostTest 'pwm-v2' $pwm @('-DESP_ARDUINO_VERSION_MAJOR=2')
	$codec = @("$root\tests\es8311_test.cpp", "$root\OTA-bot-Mochi\src\sound\buzzer_output.cpp",
		"$root\OTA-bot-Mochi\src\sound\es8311_output.cpp", "$root\OTA-bot-Mochi\src\sound\sound.cpp",
		"$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp")
	Invoke-HostTest 'es8311' $codec @('-DMOCHI_BOARD_AMOLED_18')
	Write-Output 'All host tests passed'
} finally {
	# Windows can briefly retain a lock on a just-exited test executable.
	for ($attempt = 0; $attempt -lt 5; ++$attempt) {
		try {
			Remove-Item -LiteralPath $output -Recurse -Force
			break
		} catch {
			if ($attempt -eq 4) { throw }
			Start-Sleep -Milliseconds 100
		}
	}
}

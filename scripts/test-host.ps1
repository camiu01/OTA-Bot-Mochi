# Native regression checks only. This does not compile or upload firmware.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$output = Join-Path ([System.IO.Path]::GetTempPath()) ("mochi-tests-" + [guid]::NewGuid())
New-Item -ItemType Directory -Path $output | Out-Null
# Build one native executable and stop on the first failure.
function Invoke-HostTest([string] $Name, [string[]] $Sources, [string[]] $Flags = @()) {
	# An IDE-local AMOLED selection must not change the LCD baseline fixtures.
	if ($Flags -notcontains '-DMOCHI_BOARD_AMOLED_18') { $Flags += '-DMOCHI_BOARD_LCD_169' }
	& g++ -std=c++17 -Wall -Wextra -Werror @Flags -I "$root\tests\stubs" @Sources -o "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name test build failed" }
	& "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name tests failed" }
}
try {
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
	Invoke-HostTest 'startup' @("$root\tests\startup_test.cpp")
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
	Invoke-HostTest 'face-preset' ($runtime + @("$root\tests\face_preset_test.cpp", "$root\tests\stubs\imu_noop.cpp"))
	$renderRuntime = @($runtime | Where-Object { $_ -ne "$root\tests\stubs\eyes_draw.cpp" })
	$renderEyes = @("$root\OTA-bot-Mochi\src\eyes\eyes_draw.cpp", "$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'eyes' ($renderRuntime + @("$root\tests\eyes_test.cpp") + $renderEyes)
	Invoke-HostTest 'face-preset-raster' ($renderRuntime + @("$root\tests\face_preset_raster_test.cpp") + $renderEyes)
	Invoke-HostTest 'all-faces-raster' ($renderRuntime + @("$root\tests\face_preset_raster_test.cpp") + $renderEyes) @('-O2', '-DALL_FACES_RASTER')
	Invoke-HostTest 'faces' ($renderRuntime + @("$root\tests\face_unique_test.cpp") + $renderEyes)
	Invoke-HostTest 'panel' ($renderRuntime + @("$root\tests\eye_scaling_test.cpp") + $renderEyes)
	Invoke-HostTest 'accent' ($renderRuntime + @("$root\tests\eye_accent_test.cpp") + $renderEyes)
	Invoke-HostTest 'reference' ($renderRuntime + @("$root\tests\reference_test.cpp") + $renderEyes)
	Invoke-HostTest 'performance' ($renderRuntime + @("$root\tests\performance_test.cpp") + $renderEyes)
	$uiPanel = @(
		"$root\tests\ui_panel_test.cpp",
		"$root\OTA-bot-Mochi\src\ui\screens.cpp",
		"$root\OTA-bot-Mochi\src\ui\context_views.cpp",
		"$root\OTA-bot-Mochi\src\clock\clock_ui.cpp",
		"$root\OTA-bot-Mochi\src\settings\settings_menu.cpp",
		"$root\tests\stubs\imu_noop.cpp"
	)
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

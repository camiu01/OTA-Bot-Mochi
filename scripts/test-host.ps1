# Native regression checks only. This does not compile or upload firmware.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$output = Join-Path ([System.IO.Path]::GetTempPath()) ("mochi-tests-" + [guid]::NewGuid())
New-Item -ItemType Directory -Path $output | Out-Null
# Build one native executable and stop on the first failure.
function Invoke-HostTest([string] $Name, [string[]] $Sources, [string[]] $Flags = @()) {
	& g++ -std=c++17 -Wall -Wextra -Werror @Flags -I "$root\tests\stubs" @Sources -o "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name test build failed" }
	& "$output\$Name.exe"
	if ($LASTEXITCODE -ne 0) { throw "$Name tests failed" }
}
try {
	& g++ -std=c++17 -Wall -Wextra -Werror -I "$root\tests\stubs" -fsyntax-only "$root\OTA-bot-Mochi\src\touch\touch.cpp"
	if ($LASTEXITCODE -ne 0) { throw 'Touch driver syntax check failed' }
	$runtime = @(
		"$root\OTA-bot-Mochi\src\settings\settings.cpp",
		"$root\OTA-bot-Mochi\src\settings\usb_commands.cpp",
		"$root\OTA-bot-Mochi\src\power\battery.cpp",
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
	Invoke-HostTest 'ui' ($runtime + $ui)
	Invoke-HostTest 'imu' ($runtime + @("$root\tests\imu_test.cpp", "$root\OTA-bot-Mochi\src\eyes\eyes_imu.cpp",
		"$root\OTA-bot-Mochi\src\bus\i2c_bus.cpp"))
	$settings = @("$root\tests\settings_test.cpp", "$root\OTA-bot-Mochi\src\settings\settings_menu.cpp",
		"$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'settings' ($runtime + $settings)
	$renderRuntime = @($runtime | Where-Object { $_ -ne "$root\tests\stubs\eyes_draw.cpp" })
	$render = @("$root\tests\eyes_test.cpp", "$root\OTA-bot-Mochi\src\eyes\eyes_draw.cpp",
		"$root\OTA-bot-Mochi\src\eyes\eye_shapes.cpp", "$root\OTA-bot-Mochi\src\eyes\eye_lids.cpp",
		"$root\tests\stubs\imu_noop.cpp")
	Invoke-HostTest 'eyes' ($renderRuntime + $render)
	Invoke-HostTest 'sound' ($runtime + @("$root\tests\sound_test.cpp", "$root\tests\stubs\imu_noop.cpp"))
	$pwm = @("$root\tests\pwm_test.cpp", "$root\OTA-bot-Mochi\src\power\backlight.cpp", "$root\OTA-bot-Mochi\src\sound\buzzer_output.cpp")
	Invoke-HostTest 'pwm-v3' $pwm
	Invoke-HostTest 'pwm-v2' $pwm @('-DESP_ARDUINO_VERSION_MAJOR=2')
	Write-Output 'All host tests passed'
} finally {
	Remove-Item -LiteralPath $output -Recurse -Force
}

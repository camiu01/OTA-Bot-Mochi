# Native SDL2 preview only. This does not compile or upload ESP32 firmware.
[CmdletBinding()]
param(
	[ValidatePattern('^[1-9][0-9]*x[1-9][0-9]*$')]
	[string] $Size,
	[ValidateSet('eyes', 'clock', 'route', 'alerts', 'weather', 'settings',
		'desk', 'bigclock', 'overview', 'timer', 'device', 'pong', 'analogclock')]
	[string] $Page,
	[ValidateRange(1, 2147483647)]
	[int] $Frames,
	[string] $Command,
	[string] $Screenshot,
	[switch] $Headless,
	[switch] $Rebuild,
	[switch] $TestInput,
	[string] $Compiler
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

# Keep the compiler, SDL headers, import library, and DLL in the same toolchain.
function Find-Compiler {
	if ($Compiler) {
		return (Get-Command $Compiler -CommandType Application -ErrorAction Stop).Source
	}
	$found = Get-Command g++.exe -CommandType Application -ErrorAction SilentlyContinue
	if ($found) { return $found.Source }
	foreach ($prefix in @('C:\msys64\ucrt64', 'C:\msys64\mingw64')) {
		$path = Join-Path $prefix 'bin\g++.exe'
		if (Test-Path -LiteralPath $path) { return $path }
	}
	throw 'MinGW g++ not found. Install MSYS2 GCC; see docs/TESTING.md (Windows PowerShell).'
}

if ($Headless -and !$Frames) { throw '-Headless requires -Frames so the preview exits automatically.' }
$compilerPath = Find-Compiler
$bin = Split-Path -Parent $compilerPath
$prefix = Split-Path -Parent $bin
foreach ($file in @('include\SDL2\SDL.h', 'lib\libSDL2.dll.a', 'bin\SDL2.dll')) {
	if (!(Test-Path -LiteralPath (Join-Path $prefix $file))) {
		throw "SDL2 development files missing in $prefix. Install SDL2 for this compiler; see docs/TESTING.md."
	}
}
$output = Join-Path ([System.IO.Path]::GetTempPath()) 'mochi-emulator'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$suffix = if ($Size) { "-$Size" } else { '' }
if ($TestInput) { $suffix += '-input-test' }
$binary = Join-Path $output "lcd-emulator-windows$suffix.exe"

# Keep this source list aligned with tools/run-emulator.sh.
$sources = @(
	'OTA-bot-Mochi/src/bus/i2c_bus.cpp',
	'OTA-bot-Mochi/src/sound/es8311_output.cpp',
	'OTA-bot-Mochi/src/settings/settings.cpp',
	'OTA-bot-Mochi/src/settings/usb_commands.cpp',
	'OTA-bot-Mochi/src/settings/settings_menu.cpp',
	'OTA-bot-Mochi/src/power/battery.cpp',
	'OTA-bot-Mochi/src/power/backlight.cpp',
	'OTA-bot-Mochi/src/eyes/eyes_anim.cpp',
	'OTA-bot-Mochi/src/eyes/eyes_expression.cpp',
	'OTA-bot-Mochi/src/eyes/eyes_faces.cpp',
	'OTA-bot-Mochi/src/eyes/eye_life.cpp',
	'OTA-bot-Mochi/src/eyes/eyes_draw.cpp',
	'OTA-bot-Mochi/src/eyes/eye_shapes.cpp',
	'OTA-bot-Mochi/src/eyes/eye_lids.cpp',
	'OTA-bot-Mochi/src/ui/view_state.cpp',
	'OTA-bot-Mochi/src/ui/ui_theme.cpp',
	'OTA-bot-Mochi/src/ui/ui_state.cpp',
	'OTA-bot-Mochi/src/ui/i18n.cpp',
	'OTA-bot-Mochi/src/ui/weather_sync.cpp',
	'OTA-bot-Mochi/src/ui/screens.cpp',
	'OTA-bot-Mochi/src/ui/context_views.cpp',
	'OTA-bot-Mochi/src/clock/clock_ui.cpp',
	'OTA-bot-Mochi/src/sound/sound.cpp',
	'OTA-bot-Mochi/src/sound/buzzer_output.cpp',
	'tests/stubs/imu_noop.cpp',
	'tools/lcd_emulator.cpp',
	'tools/emulator_input.cpp'
	'tools/emulator_terminal.cpp'
) | ForEach-Object { Join-Path $root $_ }
if ($TestInput) { $sources += Join-Path $root 'tests/emulator_input_test.cpp' }

function Test-Rebuild {
	if ($Rebuild -or !(Test-Path -LiteralPath $binary)) { return $true }
	$stamp = (Get-Item -LiteralPath $binary).LastWriteTimeUtc
	$inputs = @($sources) + @($PSCommandPath, $compilerPath,
		(Join-Path $root 'OTA-bot-Mochi/pin_config.h'),
		(Join-Path $prefix 'lib/libSDL2.dll.a'))
	foreach ($directory in @('OTA-bot-Mochi/src', 'tests/stubs', 'tools')) {
		$inputs += Get-ChildItem -LiteralPath (Join-Path $root $directory) -Filter '*.h' -Recurse |
			Select-Object -ExpandProperty FullName
	}
	foreach ($inputPath in $inputs) {
		if ((Get-Item -LiteralPath $inputPath).LastWriteTimeUtc -gt $stamp) { return $true }
	}
	$marker = "$binary.compiler"
	return !(Test-Path -LiteralPath $marker) -or
		([System.IO.File]::ReadAllText($marker) -ne $compilerPath)
}

$oldPath = $env:PATH
$oldDriver = $env:SDL_VIDEODRIVER
try {
	# GCC's helpers and SDL2's runtime DLL must be available to child processes.
	$env:PATH = "$bin;$oldPath"
	if (Test-Rebuild) {
		$flags = @('-std=c++17', '-Wall', '-Wextra', '-Werror',
			'-I', (Join-Path $root 'tests/stubs'),
			'-I', (Join-Path $prefix 'include/SDL2'))
		if ($Size) {
			$width, $height = $Size.Split('x')
			$flags += @("-DLCD_WIDTH=$width", "-DLCD_HEIGHT=$height")
		}
		if ($TestInput) { $flags += '-DMOCHI_EMULATOR_INPUT_TEST' }
		Write-Host 'Building Windows LCD emulator...'
		& $compilerPath @flags @sources -o $binary '-L' (Join-Path $prefix 'lib') '-lSDL2'
		if ($LASTEXITCODE -ne 0) { throw 'LCD emulator build failed.' }
		[System.IO.File]::WriteAllText("$binary.compiler", $compilerPath)
	}
	$launchArgs = @()
	if ($Page) { $launchArgs += @('--page', $Page.ToLowerInvariant()) }
	if ($Frames) { $launchArgs += @('--frames', "$Frames") }
	if ($Command) { $launchArgs += @('--command', $Command) }
	if ($Screenshot) { $launchArgs += @('--screenshot', $Screenshot) }
	if ($Headless) { $env:SDL_VIDEODRIVER = 'dummy' }
	& $binary @launchArgs
	if ($LASTEXITCODE -ne 0) { throw "LCD emulator exited with code $LASTEXITCODE." }
} finally {
	$env:PATH = $oldPath
	$env:SDL_VIDEODRIVER = $oldDriver
}

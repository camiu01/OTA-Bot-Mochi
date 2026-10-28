#!/usr/bin/env python3
"""Export native Mochi animations as GIFs; never build or upload ESP32 firmware."""
import argparse
from datetime import datetime
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[1]
STYLES = ("mochi", "bloub", "normal", "trapez")
SIZES = ("240x280", "368x448", "240x320", "320x480", "128x160", "160x128")
# Same rendering runtime as scripts/test-host.ps1 and scripts/test-host.sh, without SDL.
SOURCES = (
	"OTA-bot-Mochi/src/bus/i2c_bus.cpp",
	"OTA-bot-Mochi/src/sound/es8311_output.cpp",
	"OTA-bot-Mochi/src/settings/settings.cpp",
	"OTA-bot-Mochi/src/settings/usb_commands.cpp",
	"OTA-bot-Mochi/src/power/battery.cpp",
	"OTA-bot-Mochi/src/eyes/face_catalog.cpp",
	"OTA-bot-Mochi/src/eyes/face_dispatch.cpp",
	"OTA-bot-Mochi/src/eyes/face_primitives.cpp",
	"OTA-bot-Mochi/src/eyes/face_glyph_primitives.cpp",
	"OTA-bot-Mochi/src/eyes/eye_shapes.cpp",
	"OTA-bot-Mochi/src/eyes/eye_lids.cpp",
	"OTA-bot-Mochi/src/eyes/eyes_anim.cpp",
	"OTA-bot-Mochi/src/eyes/eyes_expression.cpp",
	"OTA-bot-Mochi/src/eyes/eyes_faces.cpp",
	"OTA-bot-Mochi/src/eyes/eye_life.cpp",
	"OTA-bot-Mochi/src/ui/view_state.cpp",
	"OTA-bot-Mochi/src/ui/ui_theme.cpp",
	"OTA-bot-Mochi/src/ui/ui_state.cpp",
	"OTA-bot-Mochi/src/ui/i18n.cpp",
	"OTA-bot-Mochi/src/ui/weather_sync.cpp",
	"OTA-bot-Mochi/src/sound/sound.cpp",
	"OTA-bot-Mochi/src/sound/buzzer_output.cpp",
	"OTA-bot-Mochi/src/eyes/eyes_draw.cpp",
	"tests/stubs/backlight.cpp",
	"tests/stubs/imu_noop.cpp",
	"tools/gif_capture.cpp",
)


def bounded(low, high):
	"""Create an argparse validator for bounded integer options."""
	def parse(text):
		try:
			value = int(text)
		except ValueError as error:
			raise argparse.ArgumentTypeError("Expected an integer") from error
		if not low <= value <= high:
			raise argparse.ArgumentTypeError(f"Choose a value from {low} to {high}")
		return value
	return parse


def parser():
	"""Expose a small CLI, with the double-click menu as an optional entry point."""
	result = argparse.ArgumentParser(description=__doc__)
	moods = result.add_mutually_exclusive_group()
	moods.add_argument("--mood", nargs="+", help="Canonical face names (default: uwu)")
	moods.add_argument("--all", action="store_true", help="Export every face")
	result.add_argument("--list", action="store_true", help="List all canonical face names")
	result.add_argument("--style", choices=STYLES, default="mochi")
	result.add_argument("--size", choices=SIZES, default="240x280")
	result.add_argument("--seconds", type=bounded(1, 30), default=6)
	result.add_argument("--fps", type=bounded(1, 30), default=30)
	result.add_argument("--from", dest="source", help="Start with this face, then capture its transition")
	result.add_argument("--output", type=Path, help="Destination directory; existing GIFs are never replaced")
	result.add_argument("--compiler", help="g++ executable, if not on PATH")
	result.add_argument("--interactive", action="store_true", help="Ask for faces, style and panel")
	return result


def interactive(args):
	"""Offer a short guided menu for either platform's launcher."""
	print("Mochi GIF export (native preview, no connected board needed)")
	print("Faces: uwu, mecha, moon, zen, daydream, ... or all. Use --list for every name.")
	value = input("Faces, separated by spaces [uwu]: ").strip().lower() or "uwu"
	args.all = value == "all"
	args.mood = None if args.all else value.replace(",", " ").split()
	args.style = input("Style: mochi/bloub/normal/trapez [mochi]: ").strip().lower() or "mochi"
	args.size = input("Panel: 240x280 LCD / 368x448 AMOLED [240x280]: ").strip() or "240x280"
	if args.style not in STYLES or args.size not in SIZES:
		raise ValueError("Unknown style or panel size")
	return args


def compiler_path(requested):
	"""Find the native compiler without downloading or installing anything."""
	candidates = [requested] if requested else [
		"g++", r"C:\msys64\ucrt64\bin\g++.exe", r"C:\msys64\mingw64\bin\g++.exe",
	]
	for candidate in candidates:
		found = shutil.which(candidate)
		if found:
			return Path(found).resolve()
	raise ValueError("g++ not found. Install a native C++17 compiler or pass --compiler PATH.")


def build_renderer(requested=None):
	"""Cache by source/header contents and compiler identity, never by stale timestamps alone."""
	compiler = compiler_path(requested)
	inputs = {ROOT / p for p in SOURCES}
	inputs.update((ROOT / "OTA-bot-Mochi/src").rglob("*.h"))
	inputs.update((ROOT / "tests/stubs").rglob("*.h"))
	inputs.add(ROOT / "OTA-bot-Mochi/pin_config.h")
	inputs.add(Path(__file__).resolve())
	digest = hashlib.sha256(str(ROOT).encode())
	digest.update(f"{compiler}:{compiler.stat().st_mtime_ns}".encode())
	for path in sorted(inputs):
		digest.update(str(path.relative_to(ROOT)).encode())
		digest.update(path.read_bytes())
	cache = Path(tempfile.gettempdir()) / "mochi-gif-export" / digest.hexdigest()[:20]
	binary = cache / ("gif-capture.exe" if os.name == "nt" else "gif-capture")
	if binary.is_file():
		return binary
	cache.mkdir(parents=True, exist_ok=True)
	flags = ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-DMOCHI_BOARD_LCD_169",
		"-I", str(ROOT / "tests/stubs")]
	env = os.environ.copy()
	env["PATH"] = str(compiler.parent) + os.pathsep + env.get("PATH", "")
	print("Building the native GIF renderer...", flush=True)
	with tempfile.TemporaryDirectory(prefix="compile-", dir=cache) as temporary:
		built = Path(temporary) / binary.name
		subprocess.run([str(compiler), *flags, *[str(ROOT / p) for p in SOURCES],
			"-o", str(built)], check=True, env=env, timeout=240)
		built.replace(binary)
	return binary


def select_moods(args, catalog):
	"""Validate the entire batch before creating any destination files."""
	selected = catalog if args.all else list(dict.fromkeys(args.mood or ["uwu"]))
	unknown = [name for name in [*selected, *([args.source] if args.source else [])] if name not in catalog]
	if unknown:
		raise ValueError("Unknown face: " + ", ".join(unknown) + ". Use --list for valid names.")
	return selected


def gif_delays(count, fps):
	"""Quantize cumulative time to GIF's 10 ms ticks without accumulated frame-rate drift."""
	boundaries = [(i * 100 + fps // 2) // fps for i in range(count + 1)]
	return [(b - a) * 10 for a, b in zip(boundaries, boundaries[1:])]


def encode_gif(raw, output, width, height, count, fps):
	"""Encode one bounded capture and refuse to replace an existing user file."""
	from PIL import Image
	frame_size = width * height * 3
	if raw.stat().st_size != frame_size * count:
		raise ValueError("Incomplete RGB capture")
	frames = []
	with raw.open("rb") as capture:
		for _ in range(count):
			frame = Image.frombytes("RGB", (width, height), capture.read(frame_size))
			frames.append(frame.convert("P", palette=Image.Palette.ADAPTIVE,
				colors=256, dither=Image.Dither.NONE))
	with output.open("xb") as target:
		try:
			frames[0].save(target, format="GIF", save_all=True, append_images=frames[1:],
				duration=gif_delays(count, fps), loop=0, disposal=1, optimize=False)
		except BaseException:
			target.close()
			output.unlink()
			raise


def export(args, binary, moods, env=None):
	"""Capture each requested face, keeping RAM and temporary RGB storage bounded per GIF."""
	from PIL import Image  # Fail before creating output when Pillow is missing.
	del Image
	width, height = map(int, args.size.split("x"))
	count = args.seconds * args.fps
	stamp = datetime.now().strftime("%Y%m%d-%H%M%S") + "-" + uuid.uuid4().hex[:6]
	directory = (args.output or ROOT / "exports/gifs" / stamp).resolve()
	prefix = f"{args.source}-to-" if args.source else ""
	paths = [directory / f"{prefix}{mood}-{args.style}-{args.size}.gif" for mood in moods]
	for path in paths:
		if path.exists():
			raise FileExistsError(f"Refusing to replace {path}")
	directory.mkdir(parents=True, exist_ok=True)
	with tempfile.TemporaryDirectory(prefix="mochi-gif-frames-") as temporary:
		raw = Path(temporary) / "frames.rgb"
		for index, (mood, path) in enumerate(zip(moods, paths), 1):
			print(f"[{index}/{len(moods)}] {path.name}", flush=True)
			subprocess.run([str(binary), mood, args.style, str(width), str(height),
				str(count), str(args.fps), args.source or "-", str(raw)], check=True, timeout=180, env=env)
			encode_gif(raw, path, width, height, count, args.fps)
	print(f"Saved {len(paths)} GIF(s) in {directory}")
	return paths


def main(argv=None):
	"""Return a nonzero exit status with an actionable message on failure."""
	args = parser().parse_args(argv)
	try:
		if args.interactive:
			args = interactive(args)
		binary = build_renderer(args.compiler)
		env = os.environ.copy()
		env["PATH"] = str(compiler_path(args.compiler).parent) + os.pathsep + env.get("PATH", "")
		catalog = subprocess.run([str(binary), "--list"], check=True, capture_output=True,
			text=True, timeout=30, env=env).stdout.splitlines()
		if args.list:
			print("\n".join(catalog))
			return 0
		export(args, binary, select_moods(args, catalog), env)
		return 0
	except ModuleNotFoundError as error:
		print(f"Missing dependency: {error.name}. Install Pillow in this Python environment; "
			"see docs/GIF_EXPORT.md.", file=sys.stderr)
	except (OSError, ValueError, subprocess.SubprocessError, EOFError) as error:
		print(f"GIF export failed: {error}", file=sys.stderr)
	except KeyboardInterrupt:
		print("GIF export cancelled.", file=sys.stderr)
	return 1


if __name__ == "__main__":
	sys.exit(main())

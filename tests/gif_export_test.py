"""Regression checks for the host-only GIF exporter: py tests/gif_export_test.py."""
import contextlib
import importlib.util
import io
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from PIL import Image, ImageSequence

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("gif_export", ROOT / "tools/export-gifs.py")
exporter = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(exporter)


class GifExportTests(unittest.TestCase):
	"""Check validation, GIF timing, pixels and non-destructive output."""

	def test_frame_timing_has_no_drift(self):
		for fps in (1, 7, 15, 24, 25, 30):
			delays = exporter.gif_delays(6 * fps, fps)
			self.assertEqual(sum(delays), 6000)
			self.assertTrue(all(delay > 0 and delay % 10 == 0 for delay in delays))

	def test_batch_validation_and_deduplication(self):
		args = exporter.parser().parse_args(["--mood", "uwu", "moon", "uwu"])
		self.assertEqual(exporter.select_moods(args, ["moon", "uwu"]), ["uwu", "moon"])
		args.source = "missing"
		with self.assertRaisesRegex(ValueError, "Unknown face"):
			exporter.select_moods(args, ["moon", "uwu"])

	def test_all_uses_catalog(self):
		args = exporter.parser().parse_args(["--all"])
		self.assertEqual(exporter.select_moods(args, ["moon", "uwu"]), ["moon", "uwu"])

	def test_invalid_cli_ranges_and_choices(self):
		for flags in (["--fps", "0"], ["--fps", "31"], ["--seconds", "0"],
			["--seconds", "31"], ["--size", "1x1"], ["--style", "other"],
			["--all", "--mood", "uwu"]):
			with self.subTest(flags=flags), contextlib.redirect_stderr(io.StringIO()):
				with self.assertRaises(SystemExit):
					exporter.parser().parse_args(flags)

	def test_guided_input(self):
		args = exporter.parser().parse_args([])
		with patch("builtins.input", side_effect=["moon,uwu", "trapez", "368x448"]):
			with contextlib.redirect_stdout(io.StringIO()):
				exporter.interactive(args)
		self.assertEqual(args.mood, ["moon", "uwu"])
		self.assertEqual((args.style, args.size), ("trapez", "368x448"))

	def test_exact_gif_pixels_timing_and_overwrite_protection(self):
		with tempfile.TemporaryDirectory() as temporary:
			root = Path(temporary)
			raw, target = root / "frames.rgb", root / "animation.gif"
			raw.write_bytes(bytes((255, 255, 255)) * 4 + bytes((255, 85, 139)) * 4)
			exporter.encode_gif(raw, target, 2, 2, 2, 30)
			with Image.open(target) as gif:
				frames = [frame.convert("RGB").copy() for frame in ImageSequence.Iterator(gif)]
				self.assertEqual(gif.info["loop"], 0)
				gif.seek(0)
				duration = sum(frame.info["duration"] for frame in ImageSequence.Iterator(gif))
			self.assertEqual(len(frames), 2)
			self.assertEqual(frames[0].getpixel((0, 0)), (255, 255, 255))
			self.assertEqual(frames[1].getpixel((0, 0)), (255, 85, 139))
			self.assertEqual(duration, sum(exporter.gif_delays(2, 30)))
			saved = target.read_bytes()
			with self.assertRaises(FileExistsError):
				exporter.encode_gif(raw, target, 2, 2, 2, 30)
			self.assertEqual(target.read_bytes(), saved)

	def test_truncated_capture_creates_no_gif(self):
		with tempfile.TemporaryDirectory() as temporary:
			root = Path(temporary)
			raw, target = root / "frames.rgb", root / "animation.gif"
			raw.write_bytes(b"short")
			with self.assertRaisesRegex(ValueError, "Incomplete RGB"):
				exporter.encode_gif(raw, target, 2, 2, 2, 30)
			self.assertFalse(target.exists())

	def test_batch_does_not_replace_existing_files(self):
		with tempfile.TemporaryDirectory() as temporary:
			root = Path(temporary)
			target = root / "uwu-mochi-240x280.gif"
			target.write_bytes(b"user file")
			args = exporter.parser().parse_args(["--output", temporary])
			with self.assertRaises(FileExistsError):
				exporter.export(args, Path("unused"), ["uwu"])
			self.assertEqual(target.read_bytes(), b"user file")


if __name__ == "__main__":
	unittest.main()

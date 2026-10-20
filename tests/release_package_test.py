#!/usr/bin/env python3
"""Release packaging checks with fake firmware, no network or device access."""
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("package_release", ROOT / "scripts/package-release.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ReleasePackageTest(unittest.TestCase):
	"""Keep both board outputs distinct and refuse incomplete builds."""

	def setUp(self):
		self.temporary = tempfile.TemporaryDirectory()
		self.addCleanup(self.temporary.cleanup)
		self.root = Path(self.temporary.name)
		self.lcd, self.amoled, self.output = (
			self.root / name for name in ("lcd", "amoled", "release")
		)
		for board in (self.lcd, self.amoled):
			board.mkdir()
			for name in ("OTA-bot-Mochi.ino.bin", "OTA-bot-Mochi.ino.merged.bin", "BUILD-INFO.txt"):
				(board / name).write_bytes(f"{board.name}: {name}".encode())
			(board / "private.log").write_text("must not be published")

	def test_named_binaries_and_checksums(self):
		MODULE.package(self.lcd, self.amoled, self.output)
		expected = {"FLASHING.txt", "SHA256SUMS.txt"}
		for board, source in (("lcd-169", self.lcd), ("amoled-18-v2", self.amoled)):
			for original, suffix in (
				("OTA-bot-Mochi.ino.bin", "app.bin"),
				("OTA-bot-Mochi.ino.merged.bin", "full.bin"),
				("BUILD-INFO.txt", "BUILD-INFO.txt"),
			):
				name = f"mochi-{board}-{suffix}"
				expected.add(name)
				self.assertEqual((source / original).read_bytes(), (self.output / name).read_bytes())
		self.assertEqual(expected, {path.name for path in self.output.iterdir()})
		lines = (self.output / "SHA256SUMS.txt").read_text().splitlines()
		self.assertEqual(7, len(lines))
		for line in lines:
			digest, name = line.split("  ", 1)
			self.assertEqual(digest, hashlib.sha256((self.output / name).read_bytes()).hexdigest())
		self.assertIn("including saved settings", (self.output / "FLASHING.txt").read_text())

	def test_missing_build_fails_before_creating_output(self):
		(self.amoled / "OTA-bot-Mochi.ino.merged.bin").unlink()
		with self.assertRaises(ValueError):
			MODULE.package(self.lcd, self.amoled, self.output)
		self.assertFalse(self.output.exists())

	def test_empty_build_fails_before_creating_output(self):
		(self.lcd / "OTA-bot-Mochi.ino.bin").write_bytes(b"")
		with self.assertRaises(ValueError):
			MODULE.package(self.lcd, self.amoled, self.output)
		self.assertFalse(self.output.exists())

	def test_existing_output_is_not_overwritten(self):
		self.output.mkdir()
		sentinel = self.output / "keep.txt"
		sentinel.write_text("existing work")
		with self.assertRaises(FileExistsError):
			MODULE.package(self.lcd, self.amoled, self.output)
		self.assertEqual("existing work", sentinel.read_text())
		self.assertEqual([sentinel], list(self.output.iterdir()))


if __name__ == "__main__":
	unittest.main()

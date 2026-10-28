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
			profile = "lcd-169" if board == self.lcd else "amoled-18-v2"
			(board / "OTA-bot-Mochi.ino.bin").write_bytes(f"MOCHI-OTA-v1:{profile}:16m-v1:END".encode())
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
		notes = (self.output / "FLASHING.txt").read_text()
		self.assertIn("first installation", notes)
		self.assertIn("verify the installed partition table", notes)
		self.assertIn("calibration", notes)
		self.assertIn("http://192.168.4.1", notes)
		self.assertIn("USB first", notes)
		for board in ("lcd-169", "amoled-18-v2"):
			self.assertIn(f"write-flash 0x0 mochi-{board}-full.bin", notes)

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

	def test_wrong_board_or_old_app_is_not_packaged(self):
		(self.amoled / "OTA-bot-Mochi.ino.bin").write_bytes(
			(self.lcd / "OTA-bot-Mochi.ino.bin").read_bytes()
		)
		with self.assertRaises(ValueError):
			MODULE.package(self.lcd, self.amoled, self.output)
		self.assertFalse(self.output.exists())

	def test_ota_partition_layout(self):
		entries = {}
		for line in (ROOT / "OTA-bot-Mochi/partitions.csv").read_text().splitlines():
			if not line or line.startswith("#"):
				continue
			name, kind, subtype, offset, size, _ = (field.strip() for field in line.split(","))
			entries[name] = (kind, subtype, int(offset, 0), int(size, 0))
		self.assertEqual(("data", "nvs", 0x9000, 0x5000), entries["nvs"])
		self.assertEqual(("data", "ota", 0xe000, 0x2000), entries["otadata"])
		self.assertEqual(("app", "ota_0", 0x10000, 0x640000), entries["app0"])
		self.assertEqual(("app", "ota_1", 0x650000, 0x640000), entries["app1"])
		end = 0x9000
		for _, _, offset, size in entries.values():
			self.assertGreaterEqual(offset, end)
			end = offset + size
		self.assertEqual(16 * 1024 * 1024, end)
		workflow = (ROOT / ".github/workflows/checks.yml").read_text()
		self.assertEqual(2, workflow.count('--build-property "upload.maximum_size=6553600"'))
		self.assertIn("board_build.partitions = OTA-bot-Mochi/partitions.csv", (ROOT / "platformio.ini").read_text())


if __name__ == "__main__":
	unittest.main()

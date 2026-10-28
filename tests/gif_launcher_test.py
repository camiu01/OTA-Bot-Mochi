"""POSIX launcher checks: python3 tests/gif_launcher_test.py."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SHELL = os.environ.get("MOCHI_TEST_SHELL") or shutil.which("sh")


@unittest.skipUnless(SHELL, "A POSIX shell is required; optionally set MOCHI_TEST_SHELL")
class GifLauncherTests(unittest.TestCase):
	"""Check quoted argument forwarding, guided mode, and failure propagation."""

	def invoke(self, launcher, arguments=(), status=0, missing=False):
		"""Use a tiny interpreter substitute so wrapper checks need no compiler or Pillow."""
		with tempfile.TemporaryDirectory(prefix="mochi launcher ") as temporary:
			stub = Path(temporary) / "python stub.sh"
			stub.write_bytes(("#!/bin/sh\nprintf '%s\\n' \"$@\"\nexit " + str(status) + "\n").encode())
			stub.chmod(0o755)
			env = os.environ.copy()
			if os.name == "nt":
				env["PATH"] = str(Path(SHELL).parent) + os.pathsep + env.get("PATH", "")
			env["PYTHON"] = "/missing/mochi-test-python" if missing else stub.as_posix()
			return subprocess.run([SHELL, (ROOT / "tools" / launcher).as_posix(), *arguments],
				cwd=temporary, env=env, text=True, capture_output=True, timeout=15)

	def test_no_arguments_selects_guided_export(self):
		result = self.invoke("export-gifs.sh")
		self.assertEqual(result.returncode, 0, result.stderr)
		lines = result.stdout.splitlines()
		self.assertTrue(lines[0].endswith("/" + ROOT.name + "/tools/export-gifs.py"))
		self.assertEqual(lines[1:], ["--interactive"])

	def test_arguments_with_spaces_and_other_working_directory(self):
		arguments = ["--mood", "uwu", "moon", "--output", "folder with spaces"]
		result = self.invoke("export-gifs.sh", arguments)
		self.assertEqual(result.returncode, 0, result.stderr)
		self.assertEqual(result.stdout.splitlines()[1:], arguments)

	def test_all_faces_forwards_options(self):
		arguments = ["--size", "368x448", "--output", "folder with spaces"]
		result = self.invoke("export-all-gifs.sh", arguments)
		self.assertEqual(result.returncode, 0, result.stderr)
		lines = result.stdout.splitlines()
		self.assertTrue(lines[1].endswith("/" + ROOT.name + "/tools/export-gifs.py"))
		self.assertEqual(lines[2:], ["--all", *arguments])

	def test_failure_status_is_preserved(self):
		for launcher in ("export-gifs.sh", "export-all-gifs.sh"):
			with self.subTest(launcher=launcher):
				result = self.invoke(launcher, ["--list"], status=23)
				self.assertEqual(result.returncode, 23, result.stderr)

	def test_missing_python_has_actionable_error(self):
		result = self.invoke("export-gifs.sh", missing=True)
		self.assertEqual(result.returncode, 127)
		self.assertIn("Python 3 not found", result.stderr)


if __name__ == "__main__":
	unittest.main()

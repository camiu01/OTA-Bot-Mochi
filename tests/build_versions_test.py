"""Check shared build pins, exact Action revisions and release cache isolation."""
import configparser
import importlib.util
from pathlib import Path
import re
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('build_versions', ROOT / 'scripts/build-versions.py')
VERSIONS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERSIONS)


class BuildVersionsTests(unittest.TestCase):
	"""Keep both build paths wired to one reviewed manifest."""

	def test_shared_pins(self) -> None:
		"""Verify CI and PlatformIO consume the shared library pins."""
		config = configparser.ConfigParser(interpolation=None)
		config.read(ROOT / 'platformio.ini')
		self.assertEqual(config['platformio']['extra_configs'], 'versions.ini')
		self.assertEqual(config['env']['platform'], 'espressif32 @ ${mochi_versions.platform}')
		for key in ('nimble', 'chronos', 'gfx', 'esp32time'):
			self.assertIn('${mochi_versions.' + key + '}', config['env']['lib_deps'])
		exported = VERSIONS.export_versions(ROOT / 'versions.ini')
		self.assertIn('ESP32_CORE_VERSION=3.3.12', exported)
		self.assertIn('NIMBLE_VERSION=2.5.2', exported)
		workflow = (ROOT / '.github/workflows/checks.yml').read_text(encoding='utf-8')
		self.assertIn('python3 scripts/build-versions.py >> "$GITHUB_ENV"', workflow)
		for variable in ('ESP32TIME_VERSION', 'NIMBLE_VERSION', 'CHRONOS_VERSION', 'GFX_VERSION'):
			self.assertIn('@${' + variable + '}', workflow)

	def test_invalid_versions(self) -> None:
		"""Reject wildcard pins, missing keys and environment-line injection."""
		manifest = (ROOT / 'versions.ini').read_text(encoding='utf-8')
		with tempfile.TemporaryDirectory() as directory:
			path = Path(directory) / 'versions.ini'
			for replacement in ('*', 'latest', '2.5.2\n evil = injected'):
				path.write_text(manifest.replace('nimble = 2.5.2', 'nimble = ' + replacement), encoding='utf-8')
				with self.assertRaises(ValueError):
					VERSIONS.export_versions(path)

	def test_release_hygiene(self) -> None:
		"""Prevent mutable Actions and release object-cache restoration."""
		workflow = (ROOT / '.github/workflows/checks.yml').read_text(encoding='utf-8')
		for revision in re.findall(r'uses: [^@\s]+@([^\s]+)', workflow):
			self.assertRegex(revision, r'^[0-9a-f]{40}$')
		cache = workflow.split('- name: Restore compiled object cache', 1)[1].split('- name: Compile,', 1)[0]
		self.assertIn("github.event_name != 'release'", cache)
		self.assertIn("!startsWith(github.ref, 'refs/tags/')", cache)
		self.assertNotIn('restore-keys:', cache)
		self.assertTrue((ROOT / '.github/dependabot.yml').is_file())


if __name__ == '__main__':
	unittest.main()

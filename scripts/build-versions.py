"""Export the shared, validated firmware version pins as GitHub environment entries."""
import configparser
from pathlib import Path
import re
import sys


def export_versions(path: Path) -> str:
	"""Read exactly the expected version keys, rejecting unpinned or injected values.

	Args:
		path: Shared INI version manifest.
	Returns:
		Newline-separated environment entries.
	Raises:
		ValueError: A key or semantic version is invalid.
		OSError: The manifest cannot be read.
	"""
	config = configparser.ConfigParser(interpolation=None)
	with path.open(encoding='utf-8') as source:
		config.read_file(source)
	keys = {
		'arduino_cli': 'ARDUINO_CLI_VERSION', 'esp32_core': 'ESP32_CORE_VERSION',
		'platform': 'PLATFORM_VERSION', 'esp32time': 'ESP32TIME_VERSION',
		'nimble': 'NIMBLE_VERSION', 'chronos': 'CHRONOS_VERSION', 'gfx': 'GFX_VERSION',
	}
	values = dict(config['mochi_versions'])
	if values.keys() != keys.keys():
		raise ValueError('Unexpected build version keys')
	if any(not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+', value) for value in values.values()):
		raise ValueError('Every build version must be pinned to an exact numeric release')
	return '\n'.join(f'{variable}={values[key]}' for key, variable in keys.items())


if __name__ == '__main__':
	sys.stdout.write(export_versions(Path(__file__).resolve().parents[1] / 'versions.ini') + '\n')

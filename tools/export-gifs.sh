#!/usr/bin/env sh
# Native GIF preview only; does not compile or upload ESP32 firmware.
set -eu
root="$(CDPATH= cd "$(dirname "$0")/.." && pwd)"
python="${PYTHON:-python3}"
if ! command -v "$python" >/dev/null 2>&1; then
	echo "Python 3 not found. Install python3 or set PYTHON to its executable." >&2
	exit 127
fi
if [ "$#" -eq 0 ]; then
	exec "$python" "$root/tools/export-gifs.py" --interactive
fi
exec "$python" "$root/tools/export-gifs.py" "$@"

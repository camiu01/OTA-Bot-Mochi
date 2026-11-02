#!/usr/bin/env sh
# Export every face in all four styles, with one output subfolder per style.
set -eu
root="$(CDPATH= cd "$(dirname "$0")" && pwd)"
echo "Exporting every face in all four styles, one folder per style..."
exec sh "$root/export-gifs.sh" --all --all-styles "$@"

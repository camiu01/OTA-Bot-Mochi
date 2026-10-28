#!/usr/bin/env sh
# Export every face as a separate native-preview GIF.
set -eu
root="$(CDPATH= cd "$(dirname "$0")" && pwd)"
echo "Exporting every Mochi face, one GIF per file..."
exec sh "$root/export-gifs.sh" --all "$@"

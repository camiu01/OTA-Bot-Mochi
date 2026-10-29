# Export bot animations as GIFs

The exporter runs Mochi's real eye renderer and animation update loop on the PC.
It uses the native test raster, not copied artwork or prerecorded GIFs. No board,
Arduino CLI, SDL, firmware compilation or upload is needed. These exports are
software previews, not physical display recordings.

## Windows setup

Install Python 3 (including the `py` launcher), native C++17 `g++` (for example
MSYS2 UCRT64 GCC), and Pillow:

```powershell
py -m pip install Pillow
```

Double-click **`tools/export-gifs.bat`**. Enter one or more face
names, separated by spaces, or `all`. Then choose a style and screen size.
Defaults are `uwu`, `mochi`, 240x280, six seconds and 30 FPS.

To see every face without typing commands, double-click **`tools/export-all-gifs.bat`**.
It exports all 135 faces into the same fresh output folder, **one named GIF file
per face**, not one combined animation. Open the files in your image viewer.
This batch can take a few minutes. The window stays open to show the output path
or any error.

The first run builds a small native renderer. Later runs reuse it until its
sources, headers, exporter or compiler change. Nothing is downloaded automatically.
Use `--compiler "C:\path\to\g++.exe"` if GCC is not found.

## Linux setup

Install Python 3, its virtual-environment support, and C++17 `g++` using your
distribution's package manager. Install Pillow in a virtual environment:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install Pillow
```

The Linux launchers provide the same guided export and one-file-per-face batch:

```sh
./tools/export-gifs.sh
./tools/export-gifs.sh --mood uwu moon --size 368x448
./tools/export-gifs.sh --mood uwu --from mecha
./tools/export-all-gifs.sh --fps 15
python3 tests/gif_export_test.py
python3 tests/gif_launcher_test.py
```

You can also run `sh tools/export-gifs.sh` or `sh tools/export-all-gifs.sh` if execute
permissions were lost when unpacking the repository. The launchers work from
other working directories, preserve quoted paths, and return failures to the shell.
Set `PYTHON=/path/to/python3` to select a different interpreter. No SDL or display
server is needed; open the exported GIFs in an image viewer.

## Examples

Run from the repository root:

```powershell
# List all canonical face names.
.\tools\export-gifs.bat --list

# One or several animations.
.\tools\export-gifs.bat --mood uwu
.\tools\export-gifs.bat --mood uwu moon zen daydream --size 368x448

# Record the outgoing background fading into a face without a backdrop.
.\tools\export-gifs.bat --mood uwu --from mecha

# Every face, with a smaller frame rate to reduce file size and encoding work.
.\tools\export-gifs.bat --all --style bloub --fps 15

# Dedicated all-faces launcher; accepts the same style, size and timing options.
.\tools\export-all-gifs.bat --size 368x448 --fps 15

# Twelve seconds includes a full cycle of the longer scenes.
.\tools\export-gifs.bat --mood pong gauge --seconds 12 --output "C:\My GIFs"
```

The Python entry point also works directly:

```sh
python tools/export-gifs.py --mood uwu moon --style mochi --size 240x280
python tests/gif_export_test.py
```

## Options and output

- `--style`: `mochi`, `bloub`, `normal`, `trapez`.
- `--size`: `240x280` (LCD), `368x448` (AMOLED), `240x320`, `320x480`,
  `128x160`, `160x128`. These select raster dimensions, not hardware drivers.
- `--seconds`: 1–30, default 6. `--fps`: 1–30, default 30.
- `--from NAME`: warm up that face for six seconds, then capture the transition.
- `--output DIRECTORY`: choose an output folder. Existing GIFs are never replaced.

By default each run creates a fresh folder under `exports/gifs/`, containing files
such as `uwu-mochi-240x280.gif` or `mecha-to-uwu-mochi-240x280.gif`.
All faces loop indefinitely in the GIF viewer. A transition recording loops the
whole clip, so its end-to-start jump is not a firmware face change.

Frames use the production 33 ms update cadence, deterministic random input, a
black background and the default white eye palette. Automatic random/IMU/shake
face selection is disabled so the chosen face stays selected; its own blink,
motion and performance colors remain active. GIF uses 256 colors and 10 ms timing
units, so exported timing and gradients are approximations. RAM and temporary RGB
storage are bounded per GIF, but larger/longer exports and `--all` take more time.

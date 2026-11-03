"""Compile profile contracts and reject invalid selections without building firmware."""
from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]


class BoardProfilesTest(unittest.TestCase):
    def compile(self, flags, source, expected=True):
        result = subprocess.run(
            ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-fsyntax-only", "-x", "c++", "-I", str(ROOT / "OTA-bot-Mochi"),
             *flags, "-"], input=source, text=True, capture_output=True)
        self.assertEqual(result.returncode == 0, expected, result.stderr)

    def test_builtin_and_custom(self):
        for profile, flag, width, height, identity in [
            ("lcd-169", "MOCHI_BOARD_LCD_169", 240, 280, 0),
            ("amoled-18-v2", "MOCHI_BOARD_AMOLED_18", 368, 448, 2),
        ]:
            source = f'''#include "pin_config.h"
static_assert(LCD_WIDTH == {width} && LCD_HEIGHT == {height});
static_assert(MOCHI_IMU_CALIBRATION_ID == {identity});
static_assert(TP_SDA == IIC_SDA && TP_SCL == IIC_SCL);
static_assert(MOCHI_HAS_SD_AUDIO <= MOCHI_HAS_ES8311);
'''
            self.compile(["-D" + flag], source)
            self.compile([f'-DMOCHI_BOARD_PROFILE="src/boards/{profile}.h"'], source)
        self.compile([], '#include "pin_config.h"\nstatic_assert(MOCHI_DISPLAY_CO5300);')

    def test_codec_without_sd(self):
        self.compile(["-DMOCHI_BOARD_AMOLED_18", "-I", str(ROOT / "tests/stubs")],
                     '''#include "pin_config.h"
#undef MOCHI_HAS_SD_AUDIO
#define MOCHI_HAS_SD_AUDIO 0
#include "src/sound/sd_audio.h"
void probe() { SdAudio sd; sd.release(); (void)sd.start(0); }
''')

    def test_conflicting_selection(self):
        self.compile(["-DMOCHI_BOARD_LCD_169", "-DMOCHI_BOARD_AMOLED_18"],
                     '#include "pin_config.h"', False)
        self.compile(["-DMOCHI_BOARD_LCD_169",
                      '-DMOCHI_BOARD_PROFILE="src/boards/lcd-169.h"'],
                     '#include "pin_config.h"', False)

    def test_invalid_capabilities(self):
        for define in ["MOCHI_DISPLAY_CO5300 1", "TP_SDA 99",
                       "MOCHI_HAS_SD_AUDIO 1", "MOCHI_IMU_CALIBRATION_ID 3"]:
            name = define.split()[0]
            source = f'''#include "src/boards/lcd-169.h"
#undef {name}
#define {define}
#include "src/boards/validate.h"
'''
            self.compile([], source, False)


if __name__ == "__main__":
    unittest.main()

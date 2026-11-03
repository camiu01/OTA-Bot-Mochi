/**
 * @file export-sounds.cpp
 * @brief Export 100 eye and 40 power melodies as microSD-ready mono PCM WAVs.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/sound/face_sounds.h"
#include "../OTA-bot-Mochi/src/sound/power_sounds.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

/** @brief Write an unsigned little-endian RIFF field. @param out Stream. @param value Field. @param bytes Field width. */
static void little(std::ostream &out, uint32_t value, unsigned bytes) {
	for (unsigned i = 0; i < bytes; ++i) { out.put(char((value >> (8 * i)) & 255)); }
}

/** @brief Render one melody using the firmware oscillator and envelopes. @param path Destination. @param notes Melody. */
static void export_wav(const std::filesystem::path &path, const SoundNote (&notes)[3]) {
	if (!PcmSequence::valid(notes, 3, 30)) { throw std::runtime_error("Invalid melody"); }
	size_t frames = size_t(notes[0].ms + notes[1].ms + notes[2].ms) * 16;
	std::vector<int16_t> pcm(frames * 2);
	PcmSequence sequence;
	sequence.start(notes, 3, 30);
	sequence.render(pcm.data(), frames);
	// Never replace user recordings in an existing export directory.
	if (std::filesystem::exists(path)) { throw std::runtime_error("Destination already exists: " + path.string()); }
	std::ofstream out(path, std::ios::binary);
	out.exceptions(std::ios::failbit | std::ios::badbit);
	out.write("RIFF", 4); little(out, uint32_t(36 + frames * 2), 4);
	out.write("WAVEfmt ", 8); little(out, 16, 4);
	little(out, 1, 2); little(out, 1, 2); little(out, 16000, 4);
	little(out, 32000, 4); little(out, 2, 2); little(out, 16, 2);
	out.write("data", 4); little(out, uint32_t(frames * 2), 4);
	for (size_t i = 0; i < frames; ++i) {
		little(out, uint16_t(int32_t(pcm[2 * i]) * 8 / 3), 2);
	}
	out.close();
}

/** @brief Export into a supplied directory, without accessing hardware. @param argc Argument count. @param argv Output directory. @return Success or failure. */
int main(int argc, char **argv) {
	if (argc != 2) { std::cerr << "Usage: export-sounds <output-directory>\n"; return 1; }
	try {
		std::filesystem::path folder = std::filesystem::path(argv[1]) / "mochi" / "sounds";
		std::filesystem::create_directories(folder);
		const char *families[] = {"bright", "warm", "soft", "bounce", "flat"};
		for (uint8_t family = 0; family < uint8_t(FaceTone::Count); ++family) {
			for (uint8_t variant = 0; variant < SOUND_FACE_VARIANTS; ++variant) {
				std::string number = (variant < 9 ? "0" : "") + std::to_string(variant + 1);
				SoundNote notes[3]; face_sound_notes(FaceTone(family), variant, notes);
				export_wav(folder / ("face-" + std::string(families[family]) + "-" + number + ".wav"),
					notes);
			}
		}
		for (bool connected : {true, false}) {
			for (uint8_t variant = 0; variant < SOUND_POWER_VARIANTS; ++variant) {
				std::string number = (variant < 9 ? "0" : "") + std::to_string(variant + 1);
				SoundNote notes[3]; power_sound_notes(connected, variant, notes);
				export_wav(folder / (std::string(connected ? "charging-" : "unplugged-") + number + ".wav"), notes);
			}
		}
		std::cout << "Exported 140 eye/power WAVs to " << folder << '\n';
	} catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}

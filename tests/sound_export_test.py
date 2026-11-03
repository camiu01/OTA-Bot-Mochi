"""Check the native eye/power WAV exporter without leaving generated test assets."""
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import wave


def main():
	"""Export and verify all 140 clips, including refusal to overwrite recordings."""
	with tempfile.TemporaryDirectory(prefix="mochi-eye-wavs-") as folder:
		subprocess.run([sys.argv[1], folder], check=True)
		files = list((Path(folder) / "mochi" / "sounds").glob("*.wav"))
		assert len(files) == 140
		digests = set()
		for family in ("face-bright", "face-warm", "face-soft", "face-bounce", "face-flat", "charging", "unplugged"):
			for variant in range(1, 21):
				path = Path(folder) / "mochi" / "sounds" / f"{family}-{variant:02}.wav"
				with wave.open(str(path), "rb") as clip:
					assert (clip.getnchannels(), clip.getsampwidth(), clip.getframerate()) == (1, 2, 16000)
					assert 0 < clip.getnframes() <= 48000
					pcm = clip.readframes(clip.getnframes())
				samples = struct.unpack(f"<{len(pcm) // 2}h", pcm)
				assert samples[0] == samples[-1] == 0
				assert 0 < max(map(abs, samples)) <= 26214
				digests.add(hashlib.sha256(pcm).digest())
		assert len(digests) == 140
		before = {file.name: file.read_bytes() for file in files}
		assert subprocess.run([sys.argv[1], folder], capture_output=True).returncode != 0
		assert before == {file.name: file.read_bytes() for file in files}
	print("All 140 eye/power WAV formats, unique melodies, bounds and overwrite protection passed")


if __name__ == "__main__":
	main()

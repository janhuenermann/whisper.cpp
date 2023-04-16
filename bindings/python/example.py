# pip install scipy numpy
import numpy as np
from scipy.io import wavfile

import pywhisper

# Load whisper
assert pywhisper.init(model_path="../../models/ggml-base.en.bin"), (
    "Failed to initialize pywhisper. Check the logs above for more details."
)

# Load audio
sample_rate, data = wavfile.read('../../samples/jfk.wav')
assert sample_rate == 16000, "Only 16kHz audio is supported"

# Normalize the audio data to the range [-1, 1]
if not np.issubdtype(data.dtype, np.floating):
    data = data.astype(np.float32) / np.iinfo(data.dtype).max

transcription = pywhisper.transcribe(data)
print(transcription)
# Should print something like:
# [(0.0, 11.0, ' And so my fellow Americans, ask not what your country can do for you, ask what you can do for your country.')]

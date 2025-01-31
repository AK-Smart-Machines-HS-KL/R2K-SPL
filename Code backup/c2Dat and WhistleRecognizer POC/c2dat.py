import numpy as np
from scipy.io import wavfile
import sys
import os

def convert_wav_to_dat(wav_file, dat_file):
    try:
        # Read the WAV file
        sample_rate, audio_data = wavfile.read(wav_file)
        print(f"Sample Rate: {sample_rate} Hz")
        
        # Check the shape and type of audio data
        if audio_data.dtype != np.int16:
            print("Converting audio data to 16-bit integers.")
            audio_data = (audio_data * 32767).astype(np.int16)  # Normalize to 16-bit range

        #convert it to mono audio
        if len(audio_data.shape) == 2:
            audio_data = audio_data.mean(axis=1).astype(np.int16)


        # Save the audio data to a .dat file
        audio_data.tofile(dat_file)
        print(f"Successfully converted {wav_file} to {dat_file}.")
        print(f"Audio Data Shape: {audio_data.shape}")
        print(f"Number of Channels: {1 if len(audio_data.shape) == 1 else audio_data.shape[1]}")

    except Exception as e:
        print(f"Error during conversion: {e}")

if __name__ == "__main__":
    # Ensure correct usage
    if len(sys.argv) != 3:
        print("Usage: python wav_to_dat_converter.py <input_wav_file> <output_dat_file>")
        sys.exit(1)
    
    # Get input and output file paths
    wav_file = sys.argv[1]
    dat_file = sys.argv[2]

    # Validate input file
    if not os.path.isfile(wav_file):
        print(f"Error: The file {wav_file} does not exist.")
        sys.exit(1)

    # Perform the conversion
    convert_wav_to_dat(wav_file, dat_file)

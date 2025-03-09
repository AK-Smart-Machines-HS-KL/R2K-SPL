import numpy as np
import matplotlib.pyplot as plt
import sounddevice as sd
from pydub import AudioSegment


# File path to the .dat file
file_path = "Whistles/a5.dat"
# file_path = "Whistles/whistle.dat"

# Known parameters of the audio file
SAMPLE_RATE = 8000  # Sampling rate in Hz
BIT_DEPTH = 16      # Bit depth (e.g., 16-bit signed integers)
NUM_CHANNELS = 1    # Number of audio channels (mono)

# Step 1: Read the raw data from the .dat file
def read_dat_file(file_path, bit_depth, num_channels):
    data_type = np.int16 if bit_depth == 16 else np.int8
    raw_data = np.fromfile(file_path, dtype=data_type)
    
    # Reshape if there are multiple channels
    if num_channels > 1:
        raw_data = raw_data.reshape(-1, num_channels)
    
    return raw_data

# Step 2: Normalize the data to the range [-1, 1]
def normalize_audio(data, bit_depth):
    max_val = 2**(bit_depth - 1)  # Maximum value for signed integers
    return data / max_val

# Step 3: Plot the waveform
def plot_waveform(audio_data, sample_rate):
    duration = len(audio_data) / sample_rate  # Total duration in seconds
    time = np.linspace(0, duration, len(audio_data))  # Time vector

    plt.figure(figsize=(12, 6))
    plt.plot(time, audio_data, color='blue', lw=0.8)
    plt.title("Audio Signal in Time Domain")
    plt.xlabel("Time (s)")
    plt.ylabel("Amplitude")
    plt.grid()
    plt.show()

# Step 3: Perform FFT and plot the frequency domain
def plot_frequency_domain(audio_data, sample_rate):
    # Compute the FFT
    fft_result = np.fft.fft(audio_data)
    fft_magnitude = np.abs(fft_result)  # Magnitude of the FFT
    
    # Generate the frequency bins
    num_samples = len(audio_data)
    frequencies = np.fft.fftfreq(num_samples, d=1/sample_rate)
    
    # Only keep the positive half of the spectrum
    positive_freqs = frequencies[:num_samples // 2]
    positive_magnitudes = fft_magnitude[:num_samples // 2]
    
    # Plot the frequency domain
    plt.figure(figsize=(12, 6))
    plt.plot(positive_freqs, positive_magnitudes, color='red', lw=0.8)
    plt.title("Audio Signal in Frequency Domain")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid()

    # Set x-axis ticks every 50 Hz
    plt.xticks(np.arange(0, max(positive_freqs) + 1, 100))

    plt.show()

#Play sound
def play_audio(audio_data, sample_rate):
    print("Playing audio...")
    sd.play(audio_data, samplerate=sample_rate)
    sd.wait()  # Wait until the audio finishes playing
    print("Audio playback finished.")

# Step 3: Perform FFT and check frequency range
def detect_whistle(audio_data, sample_rate, min_freq=400, max_freq=500, magnitude_threshold=600):
    # Compute the FFT
    fft_result = np.fft.fft(audio_data)
    fft_magnitude = np.abs(fft_result)  # Magnitude of the FFT
    
    # Generate the frequency bins
    num_samples = len(audio_data)
    frequencies = np.fft.fftfreq(num_samples, d=1/sample_rate)
    
    # Only keep the positive half of the spectrum
    positive_freqs = frequencies[:num_samples // 2]
    positive_magnitudes = fft_magnitude[:num_samples // 2]
    
    # Detect frequencies within range with magnitude above threshold
    detected = any(
        (min_freq <= f <= max_freq) and (m > magnitude_threshold)
        for f, m in zip(positive_freqs, positive_magnitudes)
    )

    # Plot the frequency domain
    plt.figure(figsize=(12, 6))
    plt.plot(positive_freqs, positive_magnitudes, color='red', lw=0.8)
    plt.title("Audio Signal in Frequency Domain")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid()
    plt.xticks(np.arange(0, max(positive_freqs) + 1, 100))
    
    # Highlight detected region
    plt.axvspan(min_freq, max_freq, color='yellow', alpha=0.3, label="Whistle Detection Zone")
    plt.legend()
    plt.show()

    # Print result
    if detected:
        print(f"🔍 Whistle detected! Frequencies in {min_freq}-{max_freq} Hz exceed {magnitude_threshold}.")
    else:
        print(f"❌ No whistle detected in {min_freq}-{max_freq} Hz range.")


# Main function to execute the steps
if __name__ == "__main__":
    # Read and process the audio data
    audio_data = read_dat_file(file_path, BIT_DEPTH, NUM_CHANNELS)
    # audio_data = read_dat_file(file_path)

    # play_audio(audio_data, SAMPLE_RATE)
    normalized_data = normalize_audio(audio_data, BIT_DEPTH)
    
    # Visualize the waveform
    
    # plot_waveform(normalized_data, SAMPLE_RATE)
    plot_frequency_domain(normalized_data, SAMPLE_RATE)
    detect_whistle(normalized_data, SAMPLE_RATE)
    print(normalized_data)

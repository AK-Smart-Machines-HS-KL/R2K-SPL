from scipy.io.wavfile import read
from scipy.fft import fft, fftfreq
import numpy as np
import matplotlib.pyplot as plt
from scipy.io.wavfile import read
from scipy.fft import fft, fftfreq
import numpy as np
import matplotlib.pyplot as plt

# Charger les fichiers audio
# Chemins vers les fichiers audio
rate7, data7 = read('C:/Users/nzeun/Desktop/ProjektR2k/R2K/triangularisation/experiment004cDIKDIK.wav')
rate8, data8 = read('C:/Users/nzeun/Desktop/ProjektR2k/R2K/triangularisation/experiment004cBIGMAMA.wav')


# Si les fichiers sont en stéréo, convertissez-les en mono
if len(data7.shape) > 1:
    data7 = np.mean(data7, axis=1)
if len(data8.shape) > 1:
    data8 = np.mean(data8, axis=1)

# Normalisez les données
data7 = data7 / np.max(np.abs(data7))
data8 = data8 / np.max(np.abs(data8))

# Calculer la FFT pour chaque fichier
freqs7 = fftfreq(len(data7), d=1/rate7)
freqs8 = fftfreq(len(data8), d=1/rate8)

fft_data7 = np.abs(fft(data7))
fft_data8 = np.abs(fft(data8))

# Limiter aux fréquences positives pour la visualisation
positive_freqs7 = freqs7[:len(freqs7)//2]
positive_fft7 = fft_data7[:len(fft_data7)//2]

positive_freqs8 = freqs8[:len(freqs8)//2]
positive_fft8 = fft_data8[:len(fft_data8)//2]

# Tracer les représentations en fréquence
plt.figure(figsize=(12, 6))
plt.plot(positive_freqs7, positive_fft7, label="Frequency Spectrum: experiment004cDIKDIK.wav", color='red', alpha=0.7)
plt.plot(positive_freqs8, positive_fft8, label="Frequency Spectrum: experiment004cBIGMAMA.wav", color='blue', alpha=0.7)
plt.title("Frequency Patterns Comparison")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Amplitude")
plt.legend()
plt.grid()
plt.show()

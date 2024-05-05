import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from scipy.signal import find_peaks

# Example list of data
# data = pd.read_csv('clean_ir.csv')
data = pd.read_csv('clean_red.csv')
# ir = data['IR'].values
red = data['RED'].values

# Calculate the FFT
# fft_result = np.fft.fft(ir)
fft_result = np.fft.fft(red)
# time = np.arange(len(ir))
time = np.arange(len(red))


# Calculate the frequencies
# freqs = np.fft.fftfreq(len(ir))
freqs = np.fft.fftfreq(len(red))

# Find peaks with magnitudes greater than 1
peaks, _ = find_peaks(np.abs(fft_result), height=6.37*1e5)
peak_freqs = freqs[peaks]
# print(peak_freqs)
# Plot the magnitude of the FFT result

# Create a low-pass filter to pass only the frequencies corresponding to the peaks
filter_freq = 0.1  # Adjust this value as needed to control the width of the passband
# filtered_ir = np.zeros_like(ir)
filtered_ir = np.zeros_like(red)
for peak_freq in peak_freqs:
    filtered_ir += np.cos(2 * np.pi * peak_freq * time)

# Plot the filtered signal in the time domain
plt.plot(time, filtered_ir)


plt.figure(figsize=(10, 6))
for i in range(len(freqs)):
    magnitude = np.abs(fft_result[i])
    if i in peaks:
        plt.plot(freqs[i], magnitude, marker='o', markersize=8, color='red', label='Peaks' if i == peaks[0] else '')
    else:
        plt.plot(freqs[i], magnitude, marker='o', markersize=5, color='blue')

plt.xlabel('Frequency')
plt.ylabel('Magnitude')
plt.title('FFT Result with Peaks')
plt.grid(True)
plt.legend()


df = pd.DataFrame(filtered_ir)
df.to_csv('filtered_red_6_37.csv', index=False)
plt.show()

from clean_data2 import bwr
import matplotlib.pyplot as plt
import pandas as pd

# Read input csv file from physionet
f = open('output1.csv', 'r')
lines = f.readlines()
f.close()

# Discard the first two lines because of header. Takes either column 1 or 2 from each lines (different signal lead)
signal = [0]*(len(lines)-2)
for i in range(len(signal)):
	signal[i] = float(lines[i+1].split(',')[2])
 

# ir = signal
red = signal

# Call the BWR method
# (baseline, ecg_out) = bwr(ir)
(baseline, ecg_out) = bwr(red)

plt.subplot(2,1,1)
# plt.plot(ir, 'b-')
plt.plot(red, 'b-')
plt.plot(baseline, 'r-')

plt.subplot(2,1,2)
plt.plot(ecg_out, 'g-')

df = pd.DataFrame(ecg_out)
df.to_csv('clean_red.csv', index=False)

plt.show()
import numpy as np
from scipy import signal
from scipy.interpolate import CubicSpline, interp1d
from biosppy.signals import ecg
import matplotlib.pyplot as plt
import pandas as pd


def butter_lowpass(cutoff, fs, order):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    coefs = signal.butter(order, normal_cutoff, btype='low', analog=False)
    return coefs[0], coefs[1]


def butter_lowpass_filtfilt(data, b, a):
    y = signal.filtfilt(b, a, data)
    return y


def remove_ecg_baseline(ecg_data, sample_rate, mode='linear', number_of_neighbors=10, seconds_to_remove=1, verbose=False):

    ecg_data = (ecg_data - ecg_data.min()) / ecg_data.ptp()
    if verbose:
        plt.plot(ecg_data, label='Normalized ECG data')
        plt.legend()
        plt.show()

    b, a = butter_lowpass(cutoff=40, fs=sample_rate, order=5)
    ecg_data_filterd = butter_lowpass_filtfilt(ecg_data, b, a)
    if verbose:
        plt.plot(ecg_data_filterd, label='filtered ECG data')
        plt.legend()
        plt.show()

    out = ecg.ecg(signal=ecg_data_filterd, sampling_rate=sample_rate, show=False)
    rpeaks = np.zeros_like(ecg_data_filterd, dtype='float')
    rpeaks[out['rpeaks']] = 1.0
    if verbose:
        plt.plot(ecg_data_filterd, label='filtered ECG data')
        plt.scatter(np.where(rpeaks == 1)[0], rpeaks[rpeaks == 1], color='r', label='rpeaks')
        plt.legend()
        plt.show()

    rpeaks_indices = np.where(rpeaks == 1)[0]
    center_indices = (rpeaks_indices[:-1] + rpeaks_indices[1:]) // 2

    if verbose:
        plt.plot(ecg_data_filterd, label='filtered ECG data')
        plt.scatter(np.where(rpeaks == 1)[0], rpeaks[rpeaks == 1], color='r', label='rpeaks')
        plt.scatter(center_indices, np.ones(len(center_indices)), color='g', label='center indices between rpeaks')
        plt.legend()
        plt.show()

    selected_samples = [ecg_data_filterd[i-number_of_neighbors:i+number_of_neighbors+1] for i in center_indices]
    mean_values = np.mean(selected_samples, axis=1)

    if mode == 'cubic_spline':
        cs = CubicSpline(center_indices, mean_values)
        interpolated_data = cs(np.arange(len(ecg_data_filterd)))
        baseline_removed_data = ecg_data_filterd - interpolated_data
        if verbose:
            plt.suptitle('Cubic Spline Interpolation')
            plt.subplot(2, 1, 1)
            plt.plot(ecg_data_filterd, label='ECG data')
            plt.plot(baseline_removed_data, label='baseline removed')
            plt.legend()
            plt.subplot(2, 1, 2)
            plt.plot(ecg_data_filterd[seconds_to_remove*sample_rate:-seconds_to_remove*sample_rate], label='ECG data trimmed')
            plt.plot(baseline_removed_data[seconds_to_remove*sample_rate:-seconds_to_remove*sample_rate], label='baseline removed trimmed')
            plt.legend()
            plt.show()
    else:
        interpolation = interp1d(center_indices, mean_values, kind='linear', bounds_error=False, fill_value='extrapolate')
        interpolated_data = interpolation(np.arange(len(ecg_data_filterd)))
        baseline_removed_data = ecg_data - interpolated_data
        if verbose:
            plt.suptitle('Linear Interpolation')
            plt.subplot(2, 1, 1)
            plt.plot(ecg_data_filterd, label='ECG data')
            plt.plot(baseline_removed_data, label='baseline removed')
            plt.legend()
            plt.subplot(2, 1, 2)
            plt.plot(ecg_data_filterd[seconds_to_remove*sample_rate:-seconds_to_remove*sample_rate], label='ECG data trimmed')
            plt.plot(baseline_removed_data[seconds_to_remove*sample_rate:-seconds_to_remove*sample_rate], label='baseline removed trimmed')
            plt.legend()
            plt.show()
    return baseline_removed_data


if __name__ == '__main__':
    
        # Read the data into a DataFrame
    data = pd.read_csv('output1.csv')
    # print(data.columns)
    # print(data['IR'].head())
    # data2 = pd.read_csv('output.csv')


    # Convert the 'Timestamp' column to datetime format
    # data['Timestamp'] = pd.to_datetime(data['Timestamp'])
    
    ir_data = data['IR']
    
    # print(len(ir_data))
    
    # print (max(ir_data))
    ir_data1 = np.array(ir_data)
    sample_rate = 250 # ecg data sampling frequency
    baseline_removed_data = remove_ecg_baseline(ir_data1, sample_rate, mode='linear', number_of_neighbors=10, seconds_to_remove=1, verbose=False)
    plt.figure(figsize=(10, 6))
    # plt.plot(ir_data, label='IR Data', color='blue')
    plt.plot(baseline_removed_data, label='Baseline Removed Data', color='red')
    # plt.xlabel('Timestamp')
    # plt.ylabel('IR Data')
    # plt.title('IR Data with and without Baseline Removal')
    plt.legend()
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()
    
    df = pd.DataFrame(baseline_removed_data)
    df.to_csv('baseline_removed_data.csv', index=False)
    
    
    
    
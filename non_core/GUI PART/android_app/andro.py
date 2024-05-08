# main.py
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg as FigureCanvas
from kivy.garden.matplotlib.backend_kivyagg import NavigationToolbar2Kivy

import serial
import threading
import numpy as np
import csv
from datetime import datetime
import os


# def butter_lowpass(cutoff, fs, order=2):
#     return butter(order, cutoff, fs=fs, btype='low', analog=False)

# def butter_lowpass_filter(data, cutoff, fs, order=2):
#     b, a = butter_lowpass(cutoff, fs, order=order)
#     y = lfilter(b, a, data)
#     return y


class DataPlotter(BoxLayout):
    def __init__(self, **kwargs):
        super(DataPlotter, self).__init__(**kwargs)
        self.orientation = 'vertical'
        self.start_time = datetime.now().strftime("%dth_%b_%Y_%H_%M_%S")
        self.serial_connection = None
        self.data_values_ir = np.array([])
        self.data_values_red = np.array([])
        self.data_values_ecg = np.array([])
        self.sensor_data_headers = ["IR", "Red", "HeartRate", "BeatAvg", "FloatAvg", "Spo2Avg", "DF_robot_heartrate", "ECG"]

        self.create_widgets()

    def create_widgets(self):
        self.label_ppg_data = Label(text='PPG Sensor Data')
        self.add_widget(self.label_ppg_data)

        self.button_start = Button(text='Start', size_hint=(None, None), size=(100, 50))
        self.button_stop = Button(text='Stop', size_hint=(None, None), size=(100, 50))
        self.button_quit = Button(text='Quit', size_hint=(None, None), size=(100, 50))

        self.add_widget(self.button_start)
        self.add_widget(self.button_stop)
        self.add_widget(self.button_quit)

        self.fig = plt.Figure()
        self.ax = self.fig.add_subplot(111)
        self.canvas = FigureCanvas(self.fig)
        self.add_widget(self.canvas)

    def start_plotting(self):
        if self.serial_connection is None:
            self.serial_connection = serial.Serial('COM3', baudrate=9600)
        self.data_thread = threading.Thread(target=self.read_serial_data)
        self.data_thread.start()

    def stop_plotting(self):
        try:
            if self.serial_connection is not None:
                self.serial_connection.close()
        finally:
            self.serial_connection = None

    def read_serial_data(self):
        while self.serial_connection is not None:
            try:
                data = self.serial_connection.readline().decode('utf-8').strip()
                if data != "No finger?":
                    if data == "Wait for valid data !":
                        continue
                    data = data.split(",")
                    self.save_data_to_csv(data)
                    self.plot_data(data[0:2], data[7])
            except Exception as e:
                print(f"Error reading serial data: {e}")

    def save_data_to_csv(self, data):
        if len(data) != 8:
            return
        try:
            file_name = "output.csv"
            with open(file_name, mode='a', newline='') as file:
                writer = csv.writer(file)

                # If the file is empty, write the header
                if file.tell() == 0:
                    header = ["Timestamp"] + self.sensor_data_headers
                    writer.writerow(header)

                # Write the timestamp and data to the file
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                row = [timestamp] + data
                writer.writerow(row)
        except Exception as e:
            print(f"Error saving data: {e}")

    def plot_data(self, value, ecg):
        if not hasattr(self, 'data_values_ir'):
            self.data_values_ir = np.array([])
        if not hasattr(self, 'data_values_red'):
            self.data_values_red = np.array([])
        if not hasattr(self, 'data_values_ecg'):
            self.data_values_ecg = np.array([])
        self.data_values_ir = np.append(self.data_values_ir, float(value[0]))
        self.data_values_red = np.append(self.data_values_red, float(value[1]))
        self.data_values_ecg = np.append(self.data_values_ecg, float(ecg))

        self.ax.clear()
        self.ax.plot(self.data_values_ir, marker='o', color='b')
        self.ax.plot(self.data_values_red, marker='o', color='r')
        self.ax.plot(self.data_values_ecg, marker='o', color='g')

        self.canvas.draw()


class DataPlotterApp(App):
    def build(self):
        return DataPlotter()


if __name__ == '__main__':
    DataPlotterApp().run()

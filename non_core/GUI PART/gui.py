import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial
import threading
import numpy as np
from LPF import butter_lowpass_filter

import csv
from datetime import datetime
import os

"""Recived Data -> IR, Red HearRate BeatAvg FloatAvg Spo2Avg DF_robot_heartrate ECG"""

class DataPlotter:
    def __init__(self, root, serial_port, baudrate):
        self.root = root
        self.root.title("PPG Sensor Data")
        self.baudrate = baudrate
        root.configure(background='light blue')
        self.serial_port = serial_port
        self.serial_connection = None
        self.data_values_ir = np.array([])
        self.data_values_red = np.array([])
        self.display = {'Heart_Rate': 0, 'SPO2': 0, 'DF': 0}
        self.conv_flag = False
        self.create_widgets()
        self.sensor_data_headers = ["IR", "Red", "HeartRate", "BeatAvg", "FloatAvg", "Spo2Avg", "DF_robot_heartrate", "ECG"]
        self.start_time = start_time
        
        

    def save_data_to_csv(self, data):
        if (len(data)!=8):
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
            print(f"Error saving data : {e}")

    def update_heart_rate_spo2_ecg(self, hr, spo2, df_heart):
        # print(f"In the update_heart_rate_spo2 -> {hr}, {spo2}")
        self.display = {'Heart_Rate': hr, 'SPO2': spo2, 'DF': df_heart}
        # print(self.display['SPO2'])
        self.spo2_label.config(text = f"SPO2 -> {self.display['SPO2']}")
        self.heart_rate_label.config(text = f"Heart Rate -> {self.display['Heart_Rate']}")
        self.df_heart_label.config(text=f"DF_Heart Rate -> {self.display['DF']}")
        
    def add_graph_to_frame1(self, title, x_data, y_data):
        fig = Figure(figsize=(4, 3), tight_layout=True)
        ax = fig.add_subplot(111)
        ax.plot(x_data, y_data, marker='o', linestyle='-', color='b')
        ax.set_title(title)
        ax.set_xlabel('X-axis')
        ax.set_ylabel('Y-axis')
        canvas = FigureCanvasTkAgg(fig, master=self.frame1)
        canvas.draw()
        canvas.get_tk_widget().pack()
        
    def add_content_to_frame1(self):
        self.fig = Figure(figsize=(12, 8), dpi=100)
        self.plot1 = self.fig.add_subplot(3, 1, 1)
        self.plot2 = self.fig.add_subplot(3, 1, 2)
        self.plot3 = self.fig.add_subplot(3, 1, 3)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.frame1)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=1)
    
    def add_content_to_frame2_top(self):
        subframe = tk.Frame(self.subframe_top, bg="lightblue", width=200, height=50)
        subframe.pack(side=tk.TOP, padx=5, pady=5)
        self.spo2_label = tk.Label(subframe, text="SPO2 -> ", font=("Helvetica", 30), bg="lightblue")
        self.spo2_label.pack(padx=5, pady=5)
        

        subframe = tk.Frame(self.subframe_top, bg="lightblue", width=200, height=50)
        subframe.pack(side=tk.TOP, padx=5, pady=5)
        self.heart_rate_label = tk.Label(subframe, text="Heart Rate -> ", font=("Helvetica", 30), bg="lightblue")
        self.heart_rate_label.pack(padx=5, pady=5)
        
        subframe = tk.Frame(self.subframe_top, bg="lightblue", width=200, height=50)
        subframe.pack(side=tk.TOP, padx=5, pady=5)
        self.df_heart_label = tk.Label(subframe, text="DF_Heart Rate -> ", font=("Helvetica", 20), bg="lightblue")
        self.df_heart_label.pack(padx=5, pady=5)
    

    def add_content_to_frame2_bottom(self):
        # Create a subframe for the buttons in the bottom subframe
        self.subframe_bottom_buttons = tk.Frame(self.subframe_bottom, bg="lightcoral", width = 200, height = 50)
        self.subframe_bottom_other = tk.Frame(self.subframe_bottom, bg="lightcoral",width=200, height = 200)
        self.subframe_bottom_buttons.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)
        self.subframe_bottom_other.pack(side="top", fill="both", expand=True)
        self.subframe_bottom_buttons.pack(side="bottom", fill="both", expand=True)
        self.start_button = ttk.Button(self.subframe_bottom_buttons, text="Start", command=self.start_plotting, style='Red.TButton')
        self.start_button.pack(side=tk.TOP, padx=5, pady=5)
        self.stop_button = ttk.Button(self.subframe_bottom_buttons, text="Stop", command=self.stop_plotting, style='Green.TButton')
        self.stop_button.pack(side=tk.TOP, padx=5, pady=5)
        self.quit_button = ttk.Button(self.subframe_bottom_buttons, text="Quit", command=[self.stop_plotting, self.root.destroy], style='Black.TButton')
        self.quit_button.pack(side=tk.TOP, padx=5, pady=5)
        self.root.style = ttk.Style()
        self.root.style.configure('Red.TButton', foreground='red', background='white', width=50, height = 30, font=("Helvetica", 30))
        self.root.style.configure('Green.TButton', foreground='green', background='white', width=50, height = 30, font=("Helvetica", 30))
        self.root.style.configure('Black.TButton', foreground='black', background='white', width=50, height=30, font=("Helvetica", 30))
    
    def add_content_to_bottom_top(self):
        return
        
    def add_content_to_frame2(self):
        # Create two subframes in frame2
        self.subframe_top = tk.Frame(self.frame2, bg="lightcoral", width=200, height=250)
        self.subframe_bottom = tk.Frame(self.frame2, bg="lightcoral", width=200, height=250)
        self.subframe_top.pack(side="top", fill="both", expand=True)
        self.subframe_bottom.pack(side="bottom", fill="both", expand=True)
        # Add content to subframes
        self.add_content_to_frame2_top()
        self.add_content_to_frame2_bottom()

    def create_widgets(self):
        self.frame1 = tk.Frame(self.root, bg="lightblue", width=600, height=500)
        self.frame2 = tk.Frame(self.root, bg="lightgreen", width=200, height=500)
        self.frame1.pack(side="left", fill="both", expand=True)
        self.frame2.pack(side="right", fill="both", expand=True)
        self.add_content_to_frame1()
        self.add_content_to_frame2()
        


    def start_plotting(self):
        if self.serial_connection is None:
            self.serial_connection = serial.Serial(self.serial_port, baudrate=self.baudrate)
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
                    self.root.after(10, self.plot_data, data[0:2], data[7])
                    # self.update_heart_rate_spo2(data[2], data[5])
                    self.root.after(10, self.update_heart_rate_spo2_ecg, data[2], data[5], data[6])
            except Exception as e:
                print(f"Error reading serial data: {e}")

    def plot_data(self, value, ecg):
        # print(value)
        if not hasattr(self, 'data_values_ir'):
            self.data_values_ir = np.array([])
        if not hasattr(self, 'data_values_red'):
            self.data_values_red = np.array([])
        if not hasattr(self, 'data_values_ecg'):
            self.data_values_ecg = np.array([])
        self.data_values_ir = np.append(self.data_values_ir, float(value[0]))
        self.data_values_red = np.append(self.data_values_red, float(value[1]))
        self.data_values_ecg = np.append(self.data_values_ecg, float(ecg))
        # print(f"max -> {np.max(self.data_values_ecg)} | min -> {np.min(self.data_values_ecg)}")
        window_size = 5
        if self.conv_flag == True:
            # self.data_values_ir = np.convolve(self.data_values_ir, np.ones(window_size)/window_size, mode='valid')
            self.data_values_ir = butter_lowpass_filter(self.data_values_ir, 75, 160)
        max_data_points = 50

        # print(f"LENGTH -> {len(self.data_values_ir)}")
        if len(self.data_values_ir) > max_data_points:
            self.data_values_ir = self.data_values_ir[-max_data_points:]
            self.conv_flag = True
        if len(self.data_values_red) > max_data_points:
            self.data_values_red = self.data_values_red[-max_data_points:]
        if len(self.data_values_ecg) > max_data_points:
            self.data_values_ecg = self.data_values_ecg[-max_data_points:]
        self.update_plots()

    def update_plots(self):
        # Clear existing plots
        self.plot1.clear()
        self.plot2.clear()
        self.plot3.clear()

        # Plot new data
        self.plot1.plot(self.data_values_ir, marker='o', color='b')
        self.plot2.plot(self.data_values_red, marker='o', color='r')
        self.plot3.plot(self.data_values_ecg, marker='o', color='r')

        # Set labels
        self.plot1.set_xlabel("Time")
        self.plot1.set_ylabel("Sensor Value IR")
        self.plot2.set_xlabel("Time")
        self.plot2.set_ylabel("Sensor Value Red")
        self.plot3.set_xlabel("Time")
        self.plot3.set_ylabel("Sensor Value ECG")

        # Annotate titles with less opacity
        self.annotate_title(self.plot1, "PPG Sensor Data (IR)")
        self.annotate_title(self.plot2, "PPG Sensor Data (Red)")
        self.annotate_title(self.plot3, "ECG data")

        # Redraw the canvas
        self.canvas.draw()

    def annotate_title(self, ax, title):
    # Annotate title with less opacity
        ax.text(0.5, 0.5, title, transform=ax.transAxes, ha='center', va='center', fontsize=12, color='blue', alpha=0.5)



def name_file(self):
    self.filename = f"{self.start_time}___{self.formatted_end_datetime}"
if __name__ == "__main__":
    file_name = "output.csv"
    if os.path.exists(file_name):
        os.remove(file_name)
        print(f"File '{file_name}' deleted.")
    root = tk.Tk()
    print ("Code_started")
    current_datetime = datetime.now()
    start_time = current_datetime.strftime("%dth_%b_%Y_%H_%M_%S")
    app = DataPlotter(root, serial_port='COM3', baudrate=9600)
    try:
        root.protocol("WM_DELETE_WINDOW", lambda: [app.stop_plotting(), root.destroy()])
        root.mainloop()
    except KeyboardInterrupt:
        print("KeyBoard Interupt")
        app.stop_plotting()
        app.root.destroy()
    end_datetime = datetime.now()
    end_time = end_datetime.strftime("%dth_%b_%Y_%H_%M_%S")
    filename = f"{start_time}___{end_time}"
    # Specify the current and new file names
    current_file_name = 'output.csv'
    new_file_name = filename

    # Check if the current file exists before renaming
    if os.path.exists(current_file_name):
        # Rename the file
        os.rename(current_file_name, new_file_name)
        print(f"File '{current_file_name}' has been renamed to '{new_file_name}'.")
    else:
        print(f"Error: File '{current_file_name}' not found.")

import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial
import threading
import numpy as np

class DataPlotter:
    def __init__(self, root, serial_port, baudrate):
        self.root = root
        self.root.title("PPG Sensor Data")
        self.baudrate = baudrate
        root.configure(background='light blue')

        self.serial_port = serial_port
        self.serial_connection = None

        self.create_widgets()

    def update_heart_rate_spo2(self, hr, spo2):
        self.info_text1.set(f"Heart Rate -> {hr}")
        self.info_text2.set(f"SPO2 -> {spo2}")

    def create_widgets(self):
        self.fig = Figure(figsize=(12, 6), dpi=100)  # Increased width to accommodate three subplots
        self.plot1 = self.fig.add_subplot(3, 1, 1)
        self.plot2 = self.fig.add_subplot(3, 1, 2)
        self.plot3 = self.fig.add_subplot(3, 1, 3)

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Additional Info 1
        frame_info1 = tk.Frame(self.canvas_widget)
        self.info_text1 = tk.StringVar(value="Additional Info 1: ")
        self.label_info1 = tk.Label(frame_info1, textvariable=self.info_text1)
        self.label_info1.pack(side=tk.LEFT, anchor=tk.SW, padx=5, pady=5)
        window_info1 = self.canvas_widget.create_window(10, 900, anchor=tk.NW, window=frame_info1)

        # Additional Info 2
        frame_info2 = tk.Frame(self.canvas_widget)
        self.info_text2 = tk.StringVar(value="Additional Info 2: ")
        self.label_info2 = tk.Label(frame_info2, textvariable=self.info_text2)
        self.label_info2.pack(side=tk.RIGHT, anchor=tk.SE, padx=5, pady=5)
        window_info2 = self.canvas_widget.create_window(10, 980, anchor=tk.NW, window=frame_info2)

        self.start_button = ttk.Button(self.root, text="Start", command=self.start_plotting, style='Red.TButton')
        self.start_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.stop_button = ttk.Button(self.root, text="Stop", command=self.stop_plotting, style='Green.TButton')
        self.stop_button.pack(side=tk.RIGHT, padx=5, pady=5)

        self.quit_button = ttk.Button(self.root, text="Quit", command=[self.root.destroy, self.stop_plotting], style='Black.TButton')
        self.quit_button.pack(side=tk.BOTTOM, padx=5, pady=5)

        self.root.style = ttk.Style()
        self.root.style.configure('Red.TButton', foreground='red', background='white', width=15)
        self.root.style.configure('Green.TButton', foreground='green', background='white', width=15)
        self.root.style.configure('Black.TButton', foreground='black', background='white', width=15)

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
                if (data != "No finger?"):
                    data = data.split(",")
                    self.root.after(10, self.plot_data, data[0:3])
                    self.update_heart_rate_spo2(data[2], data[5])
            except Exception as e:
                print(f"Error: {e}")

    def plot_data(self, value):
        print(value)
        if not hasattr(self, 'data_values_ir'):
            self.data_values_ir = np.array([])
        if not hasattr(self, 'data_values_red'):
            self.data_values_red = np.array([])
        if not hasattr(self, 'data_values_hear_rate'):
            self.data_values_hear_rate = np.array([])

        self.data_values_ir = np.append(self.data_values_ir, float(value[0]))
        self.data_values_red = np.append(self.data_values_red, float(value[1]))
        self.data_values_hear_rate = np.append(self.data_values_hear_rate, float(value[2]))
        max_data_points = 50
        # correlation_value = np.corrcoef(self.data_values_ir, self.data_values_red)[0, 1]

        print(f"LENGTH -> {len(self.data_values_ir)}")
        if len(self.data_values_ir) > max_data_points:
            self.data_values_ir = self.data_values_ir[-max_data_points:]
        if len(self.data_values_red) > max_data_points:
            self.data_values_red = self.data_values_red[-max_data_points:]
        if len(self.data_values_hear_rate) > max_data_points:
            self.data_values_hear_rate = self.data_values_hear_rate[-max_data_points:]
        # correlation_label_text = f"Correlation: {correlation_value:.2f}"
        # if hasattr(self, 'correlation_label'):
        #     self.correlation_label.config(text=correlation_label_text)
        # else:
        #     self.correlation_label = tk.Label(self.root, text=correlation_label_text)
        # self.correlation_label.pack(side=tk.BOTTOM, padx=5, pady=5)
        
        

        self.plot1.clear()
        self.plot1.plot(self.data_values_ir, marker='o', color='b')
        self.plot1.set_title("PPG Sensor Data (IR)")
        self.plot1.set_xlabel("Time")
        self.plot1.set_ylabel("Sensor Value IR")

        self.plot2.clear()
        self.plot2.plot(self.data_values_red, marker='o', color='r')
        self.plot2.set_title("PPG Sensor Data (Red)")
        self.plot2.set_xlabel("Time")
        self.plot2.set_ylabel("Sensor Value Red")
        
        
        self.plot3.clear()
        self.plot3.plot(self.data_values_hear_rate, marker='o', color='r')
        self.plot3.set_title("PPG Sensor Data (Heart Rate)")
        self.plot3.set_xlabel("Time")
        self.plot3.set_ylabel("Sensor Value Heart Rate")
        self.canvas.draw()
        
        # self.canvas.draw()

if __name__ == "__main__":
    root = tk.Tk()
    app = DataPlotter(root, serial_port='COM5', baudrate=9600)
    try:
        root.protocol("WM_DELETE_WINDOW", lambda: [app.stop_plotting(), root.destroy()])
        root.mainloop()
    except KeyboardInterrupt:
        print("KeyBoard Interupt")

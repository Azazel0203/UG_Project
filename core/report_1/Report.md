Progress Update:

1. Hardware Integration:

   - Successfully integrated two PPG (Photoplethysmography) sensors and an ECG (Electrocardiogram) sensor with the ESP32 microcontroller.
   - Defined pin assignments and established communication protocols between the sensors and the ESP32.
2. Sensor Data Acquisition:

   - Modified the existing code to accommodate data acquisition from multiple sensors.
   - Implemented logic for collecting 2 PPG and ECG data simultaneously.
3. Display Integration:

   - Extended OLED display functionality to visualize PPG waveforms, ECG signals, heart rate, and SpO2 readings.
   - Updated OLED display layout to accommodate multiple sensor data visualizations.
4. Blynk Integration:

   - Expanded Blynk integration to include real-time visualization of PPG waveforms, ECG signals, heart rate, and SpO2 readings on the Blynk app.
   - Configured Blynk widgets to display sensor data and enable remote monitoring and control.
5. Data Logging to SD Card:

   - Currently in progress. Developing functionality to log sensor data (PPG, ECG, heart rate, SpO2) to an SD card for offline analysis.
   - Planning to implement file management and data retrieval features for efficient data logging.
6. GUI Development:

   - Implemented a GUI using libraries such as Tkinter in Python or similar frameworks compatible with ESP32 microcontroller.
   - Designed an intuitive interface with buttons for starting, stopping, and quitting monitoring sessions.
7. Data Plotting:

   - Developed functionality to plot real-time ECG data on the GUI interface.
   - Integrated graph plotting libraries like Matplotlib to visualize ECG waveforms.
8. Data Processing with Butterworth Low-Pass Filter:

   - Implemented Butterworth low-pass filter for ECG data processing to remove noise and artifacts.
   - Incorporated digital signal processing techniques to enhance the quality of ECG signals.
9. Data Logging and File Management:

   - Developed data logging functionality to record ECG data to a file system.
   - Implemented file management features for organizing and storing ECG data files.
10. Start/Stop/Quit Buttons:

    - Integrated start, stop, and quit buttons on the GUI interface for user interaction.
    - Programmed button functionalities to initiate, pause, and terminate ECG monitoring sessions.

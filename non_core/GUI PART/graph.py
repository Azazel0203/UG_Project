import streamlit as st
import serial,time,csv,os
import time
import numpy as np

length = 100
ser = serial.Serial('COM5',baudrate=115200)


# Function to generate random data for the example
def get_data():
    line = ser.readline()
    data = (line[0:-2]).decode("utf-8").split(',')
    print(data)
    return data[0]
# Streamlit app


def main():
    st.title("Real-time Graph Example")
    data = []
    start = False
    chart_placeholder = st.line_chart(data)
    while True:
        try:        
            if start == False:
                curr_line = ser.readline()
                if curr_line[0:-2]==b'START':
                    start = True
                    print("Program Start")
                    continue
                else:
                    continue
            new_data = get_data()
            data.append(new_data)
            data = data[-length:]
            chart_placeholder.line_chart(data)
            time.sleep(0.01)
        except KeyboardInterrupt:
            break 


if __name__ == "__main__":
    main()

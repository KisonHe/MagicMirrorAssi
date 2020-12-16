import serial
import time

def recieve(parameter_list):
    """
    docstring
    """
    pass

def send(parameter_list):
    """
    docstring
    """
    pass

if (__name__ == "__main__"):
    ser = serial.Serial("/dev/ttyS0")
    ser.baudrate = 115200
    ser.open()
    # if (not ser.is_open):
        


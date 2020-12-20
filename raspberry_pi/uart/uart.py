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
    # ser = serial.Serial("/dev/ttyS0")
    ser = serial.Serial("/dev/ttyUSB0")
    ser.baudrate = 115200
    ser.timeout = 5
    ser.close()
    ser.open()
    while (1):
        s = ser.read(100)
        print(type(s))

    
    # if (not ser.is_open):
        


import serial

# 9600 is the baudrate, should match serial baudrate in arduino
serial_port = serial.Serial('/dev/ttyACM0', 9600)

while True:
    print(serial_port.readline())
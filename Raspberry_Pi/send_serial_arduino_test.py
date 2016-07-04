import serial

stop = 0
forward = 1
backward = 2
right = 3
left = 4

# 9600 is the baudrate, should match serial baudrate in arduino
serial_port = serial.Serial('/dev/ttyACM0', 9600) 

while True:
  serial_port.write(str(forward))
import serial

# 9600 is the baudrate, should match serial baudrate in arduino
serial_port = serial.Serial('/dev/ttyACM0', 9600)

# Make the led on arduino to blink 2 times
while True:
    serial_port.write('2')



    

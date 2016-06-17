# WRITTEN IN PYTHON3

# RASPBERRY PI (CLIENT)

import socket

TCP_IP = '172.24.1.62' # The PC's IP address! (maybe not?)
TCP_PORT = 5005
BUFFER_SIZE = 1024	# Maximum no of bytes(?) to be received at the same time
MESSAGE = "Hello world!"

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((TCP_IP, TCP_PORT))
client_socket.sendall(MESSAGE.encode('utf-8'))	# Convert "MESSAGE" to the corresponding sequence of bytes (we always transmit data in the form of bytes/bits) 
data = client_socket.recv(BUFFER_SIZE)
client_socket.close()

print("Received data: " + data.decode('utf-8'))	# Convert "data", a sequence of bytes, back to text 
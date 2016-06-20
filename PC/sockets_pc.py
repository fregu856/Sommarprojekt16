# WRITTEN IN PYTHON3

# PC (CLIENT)

import socket

TCP_IP = '172.24.1.62' # The PC's IP address! (maybe not?)
TCP_PORT = 5005
BUFFER_SIZE = 1024	# Maximum no of bytes(?) to be received at the same time

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)	# TCP-socket
client_socket.connect((TCP_IP, TCP_PORT))

data = client_socket.recv(BUFFER_SIZE)
client_socket.close()

print("Received data: " + data.decode('utf-8'))	# Convert "data", a sequence of bytes, back to text 
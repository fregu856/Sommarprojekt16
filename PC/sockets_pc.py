# WRITTEN IN PYTHON3

# PC (CLIENT)

import socket

TCP_IP = '169.254.74.215' # The RPI's IP address! 
TCP_PORT = 5005
BUFFER_SIZE = 1024	# Maximum no of bytes(?) to be received at the same time

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)	# TCP-socket
client_socket.connect((TCP_IP, TCP_PORT))

data = client_socket.recv(BUFFER_SIZE)
client_socket.close()

print("Received data: " + data.decode('utf-8'))	# Convert "data", a sequence of bytes, back to text 
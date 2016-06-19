# WRITTEN IN PYTHON3

# RASPBERRY PI (SERVER)

import socket

TCP_IP = '169.254.74.215' # The RPI's IP address!
TCP_PORT = 5005
BUFFER_SIZE = 1024
MESSAGE = "Hello world!"

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)	# TCP-socket
server_socket.bind((TCP_IP, TCP_PORT))
server_socket.listen(5)	# 5 = no of unaccepted connections that the system will allow before refusing new connections

connection, address = server_socket.accept()
print("Connection address: " + str(address))

connection.sendall(MESSAGE.encode('utf-8'))	# Convert "MESSAGE" to the corresponding sequence of bytes (we always transmit data in the form of bytes/bits) 
connection.close()
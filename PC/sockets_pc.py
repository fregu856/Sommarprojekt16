# WRITTEN IN PYTHON3

# PC (SERVER)

import socket

TCP_IP = '172.24.1.62' # The PC's IP address! (maybe not?)
TCP_PORT = 5005
BUFFER_SIZE = 1024

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((TCP_IP, TCP_PORT))
server_socket.listen(5)	# 5 = no of unaccepted connections that the system will allow before refusing new connections
connection, address = server_socket.accept()
print("Connection address: " + str(address))

while True:
	data = connection.recv(BUFFER_SIZE)
	if not data:	# (data == 0 when transaction is done)	
		break	# Break when all data is read
	print("Received data: " + data.decode('utf-8')) # Convert "data", a sequence of bytes, back to text
	connection.send(data)	# Echo the data back to the client
	
connection.close()
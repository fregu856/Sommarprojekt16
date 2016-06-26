# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import numpy as np
import socket
import time

####################################################################

UDP_IP = '172.24.1.62' # The PC's IP address! (maybe not?) 
UDP_PORT = 5005
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	# Skapa UDP-socket

##########################################################################
# initialize the camera and grab a reference to the raw camera capture
camera_x = 40
camera_y = 40
camera = PiCamera()
camera.resolution = (camera_x, camera_y)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(camera_x, camera_y))
 
# allow the camera to warmup
time.sleep(0.1)

counter = 0
 
# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image, then initialize the timestamp
	# and occupied/unoccupied text
	print("New loop")
	image = frame.array
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	ok, image_jpg = cv2.imencode(".jpg", image)

	key = cv2.waitKey(1) & 0xFF
 
	# clear the stream in preparation for the next frame
	rawCapture.truncate(0)
 
	# if the `q` key was pressed, break from the loop
	if key == ord("q"):
		break	
		
	#number_of_digits = len(str(counter)) # Antar att number_of_digits < 10 (sa att repr. av en byte)	
	#print(str(number_of_digits) + str(counter))
	#client_socket.sendall(str(number_of_digits) + str(counter)) # str() gor om varje siffra till 1 byte!
	#uint8_counter = np.uint8(counter)	# Gor om till uint8 (en byte) for sandning
	#print(str(uint8_counter))
	#client_socket.sendto(uint8_counter, (UDP_IP, UDP_PORT))	# Skicka byte:n till PC:n
	#counter += 1
	image_array = []
	for row in image_jpg:
		for entry in image_jpg[row]:
			#print entry[0]
			uint8_value = np.uint8(entry[0])
			image_array.append(uint8_value)
			#uint8_value = np.uint8(image[row_index][col_index][0])
			#client_socket.sendto(uint8_value, (UDP_IP, UDP_PORT))
			#print(uint8_value)
			
			#uint8_value = np.uint8(image[row_index][col_index][1])
			#client_socket.sendto(uint8_value, (UDP_IP, UDP_PORT))
			#print(uint8_value)
			
			#uint8_value = np.uint8(image[row_index][col_index][2])
			#client_socket.sendto(uint8_value, (UDP_IP, UDP_PORT))
			#print(uint8_value)
	print(len(image_array))
	trans_size = np.uint32(len(image_array))
	#client_socket.sendto(trans_size, (UDP_IP, UDP_PORT))
	for entry in image_array:
		#print(entry)
		client_socket.sendto(entry, (UDP_IP, UDP_PORT))
				
	time.sleep(10)		# Delay:a 10 sek
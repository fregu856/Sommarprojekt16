# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import numpy as np
import socket

####################################################################

UDP_IP = '172.24.1.62' # The PC's IP address! (maybe not?) 
UDP_PORT = 5005
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	# Skapa UDP-socket

##########################################################################
# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(640, 480))
 
# allow the camera to warmup
time.sleep(0.1)

counter = 0
 
# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image, then initialize the timestamp
	# and occupied/unoccupied text
	image = frame.array

	hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

	lower_red = np.array([30, 150, 50])
	upper_red = np.array([255, 255, 180])

	range_mask = cv2.inRange(hsv, lower_red, upper_red)
	result = cv2.bitwise_and(image, image, mask = range_mask)

	# show the frame
	#cv2.imshow("image", image)
	#cv2.imshow("range_mask", range_mask)
	#cv2.imshow("result", result)

	key = cv2.waitKey(1) & 0xFF
 
	# clear the stream in preparation for the next frame
	rawCapture.truncate(0)
 
	# if the `q` key was pressed, break from the loop
	if key == ord("q"):
		break	
		
	#number_of_digits = len(str(counter)) # Antar att number_of_digits < 10 (sa att repr. av en byte)	
	#print(str(number_of_digits) + str(counter))
	#client_socket.sendall(str(number_of_digits) + str(counter)) # str() gor om varje siffra till 1 byte!
	uint8_counter = np.uint8(counter)	# Gor om till uint8 (en byte) for sandning
	print(str(uint8_counter))
	client_socket.sendto(uint8_counter, (UDP_IP, UDP_PORT))	# Skicka byte:n till PC:n
	counter += 1		
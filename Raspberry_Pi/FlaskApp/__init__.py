from flask import Flask, render_template, request, redirect, url_for, session, flash
from dbconnect import connect
import gc   # Garbage collection
from datetime import date
import numbers
from flask_socketio import SocketIO, emit
import serial

from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import numpy as np
import time

from threading import Thread

# stop = 0
# forward = 1
# backward = 2
# right = 3
# left = 4
# manual = 5
# auto = 6
# Kp and kd = 7
# Only Kp = 8
# Only Kd = 9


# 9600 is the baudrate, should match serial baudrate in arduino
serial_port = serial.Serial("/dev/ttyACM0", 9600) 

app = Flask(__name__)
socketio = SocketIO(app, async_mode = "threading") # Without "async_mode = "threading", sending stuff to the cliend (via socketio) doesn't work!

def check_parameter_input(value):    
    if value:
        try:
            value = int(value)
        except:
            value = "" # value must be an integer! (so don't send the parameter)
            return value
        else: # if conversion was successful:
            if value < 0:
                value = "" # value must be positive! (so don't send the parameter)
                return value
            else: # if positive integer
                return value  
            
    else: # if value == 0 or value is empty (if the parameter field was left empty)
        return value

def video_thread():
    camera = PiCamera()
    camera.hflip = True # | Rotate 180 deg if mounted upside down
    camera.vflip = True # |
    camera.resolution = (640, 480)
    camera.framerate = 32
    rawCapture = PiRGBArray(camera, size=(640, 480))
    print("Start") 
    # allow the camera to warmup
    time.sleep(0.1)
    
    # capture frames from the camera
    for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        # grab the raw NumPy array representing the image, then initialize the timestamp
        # and occupied/unoccupied text
        image = frame.array
        
        cv2.imwrite("/var/www/FlaskApp/FlaskApp/static/images/image.jpg", image)
        socketio.emit("video_test", {"data": "Test"})
     
        # clear the stream in preparation for the next frame
        rawCapture.truncate(0)
        
        time.sleep(0.1)		# Delay 0.05 sec (20 Hz)

@app.route("/")   
@app.route("/index")
def index():
    try:
        thread = Thread(target = video_thread)
        thread.start()
        return render_template("index.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
          
@app.route("/phone")
def phone():
    try:
        thread = Thread(target = video_thread)
        thread.start()
        return render_template("phone.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))        
        
@socketio.on("my event")
def handle_my_custom_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
       
@socketio.on("button_event")
def handle_button_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"])
    read_line = serial_port.readline()
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)

@socketio.on("touch_event")
def handle_touch_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"])
    read_line = serial_port.readline()
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)

@socketio.on("key_event")
def handle_key_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"])
    read_line = serial_port.readline()
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    print(read_line)
    
@socketio.on("parameters_event")
def handle_parameters_event(sent_dict):
    Kp_input = sent_dict["Kp"]
    Kd_input = sent_dict["Kd"]
    
    print("Recieved Kp: " + Kp_input)
    print("Recieved Kd: " + Kd_input)
    
    Kp_input = check_parameter_input(Kp_input) # After this, Kp_input is non-empty only if the user typed a positive integer into the Kp field (and thus we should send it to the arduino)
    Kd_input = check_parameter_input(Kd_input)
        
    if (Kp_input or Kp_input == 0) and (Kd_input or Kd_input == 0): # if valid, non-empty input for both Kp and Kd: send both Kp and Kd
        serial_port.write("7") # 7 = Kp and Kd
        serial_port.write(str(Kp_input))
        serial_port.write(str(Kd_input))
        print("New Kp and Kd sent!")
        read_line = serial_port.readline()
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
    elif Kp_input or Kp_input == 0: # if only Kp:
        serial_port.write("8") # 8 = Just Kp
        serial_port.write(str(Kp_input))
        print("New Kp sent!")
        read_line = serial_port.readline()
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
    elif Kd_input or Kd_input == 0: # if only Kd:
        serial_port.write("9") # 9 = Just Kd
        serial_port.write(str(Kd_input))
        print("New Kd sent!")
        read_line = serial_port.readline()
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        print(read_line)
        
@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == '__main__':
    socketio.run(app, "169.254.74.215", 80)
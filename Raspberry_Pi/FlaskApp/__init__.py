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

#stop = 0
#forward = 1
#backward = 2
#right = 3
#left = 4

# 9600 is the baudrate, should match serial baudrate in arduino
#serial_port = serial.Serial('/dev/ttyACM0', 9600) 

app = Flask(__name__)
socketio = SocketIO(app, async_mode = "threading")

def video_thread():
    camera = PiCamera()
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
    #print("Recieved message: " + sent_dict["data"])
    pass
       
@socketio.on("button_event")
def handle_button_event(sent_dict):
    #print("Recieved message: " + sent_dict["data"])
    #serial_port.write(sent_dict["data"])
    socketio.emit("video_test", {"data": "Test"})

@socketio.on("touch_event")
def handle_touch_event(sent_dict):
    #print("Recieved message: " + sent_dict["data"])
    #serial_port.write(sent_dict["data"])
    pass

@socketio.on("key_event")
def handle_key_event(sent_dict):
    #print("Recieved message: " + sent_dict["data"])
    #serial_port.write(sent_dict["data"])
    pass

@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == '__main__':
    socketio.run(app, "169.254.74.215", 80)
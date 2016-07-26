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

IR_0 = 0
IR_1 = 0
IR_2 = 0
IR_3 = 0
IR_4 = 0
IR_Yaw_right = 0
IR_Yaw_left = 0
Yaw = 0
p_part = 0
alpha = 0
AUTO_STATE = 0

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
        socketio.emit("video_test", {"IR_0": IR_0, "IR_1": IR_1, "IR_2": IR_2, "IR_3": IR_3, "IR_4": IR_4, "IR_Yaw_right": IR_Yaw_right, "IR_Yaw_left": IR_Yaw_left, "Yaw": Yaw, "p_part": p_part, "alpha": alpha, "AUTO_STATE": AUTO_STATE})
     
        # clear the stream in preparation for the next frame
        rawCapture.truncate(0)
        
        time.sleep(0.1) # Delay 0.1 sec (~ 0 Hz)
        
def serial_thread():
    # all global variables this function can modify:
    global IR_0, IR_1, IR_2, IR_3, IR_4, IR_Yaw_right, IR_Yaw_left, Yaw, p_part, alpha, AUTO_STATE

    while 1:
        no_bytes_waiting = serial_port.inWaiting()
        if no_bytes_waiting > 13: # the ardu sends 13 bytes at the time (11 data, 2 control)
            # read the first byte (read 1 byte): (ord: gives the actual value of the byte)
            first_byte = ord(serial_port.read(size = 1)) 
            
            # read all data bytes if first byte was the start byte:
            if first_byte == 100:
                serial_data = []
                # read all data bytes:
                for counter in range(11): # 11 data bytes is sent from the ardu
                    serial_data.append(ord(serial_port.read(size = 1)))
                # read last byte:
                last_byte = ord(serial_port.read(size = 1)) 
                
                # update the variables with the read serial data only if the last byte was the end byte:
                if last_byte == 200:
                    IR_0 = serial_data[0]
                    IR_1 = serial_data[1]
                    IR_2 = serial_data[2]
                    IR_3 = serial_data[3]
                    IR_4 = serial_data[4]
                    IR_Yaw_right = serial_data[5]
                    IR_Yaw_left = serial_data[6]
                    Yaw = serial_data[7]
                    p_part = serial_data[8]
                    alpha = serial_data[9]
                    AUTO_STATE = serial_data[10]
                else: # if final byte doesn't match: something weird has happened during transmission: flush input buffer and start over
                    serial_port.flushInput()
                    print("Something went wrong in the transaction: final byte didn't match!")                  
            else: # if first byte isn't the start byte: we're not in sync: just read the next byte until we get in sync (until we reach the start byte)
                pass
        else: # if not enough bytes for entire transmission, just wait for more data:
            pass

        time.sleep(0.025) # Delay for ~40 Hz loop frequency (faster than the sending frequency)
 
@app.route("/")   
@app.route("/index")
def index():
    try:
        thread_video = Thread(target = video_thread)
        thread_video.start()
        thread_serial = Thread(target = serial_thread)
        thread_serial.start()
        return render_template("index.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
          
@app.route("/phone")
def phone():
    try:
        thread_video = Thread(target = video_thread)
        thread_video.start()
        thread_serial = Thread(target = serial_thread)
        thread_serial.start()
        return render_template("phone.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))        
        
@socketio.on("my event")
def handle_my_custom_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
       
@socketio.on("button_event")
def handle_button_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"] + "\n") # "\n" is used as a delimiter char when the arduino reads the serial port


@socketio.on("touch_event")
def handle_touch_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"] + "\n")

@socketio.on("key_event")
def handle_key_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"] + "\n")
    
@socketio.on("parameters_event")
def handle_parameters_event(sent_dict):
    Kp_input = sent_dict["Kp"]
    Kd_input = sent_dict["Kd"]
    
    print("Recieved Kp: " + Kp_input)
    print("Recieved Kd: " + Kd_input)
    
    Kp_input = check_parameter_input(Kp_input) # After this, Kp_input is non-empty only if the user typed a positive integer into the Kp field (and thus we should send it to the arduino)
    Kd_input = check_parameter_input(Kd_input)
        
    if (Kp_input or Kp_input == 0) and (Kd_input or Kd_input == 0): # if valid, non-empty input for both Kp and Kd: send both Kp and Kd
        serial_port.write("7" + "\n" + str(Kp_input) + "\n" + str(Kd_input) + "\n")
        print("New Kp and Kd sent!")

    elif Kp_input or Kp_input == 0: # if only Kp:
        serial_port.write("8" + "\n" + str(Kp_input) + "\n") # 8 = Just Kp
        print("New Kp sent!")

    elif Kd_input or Kd_input == 0: # if only Kd:
        serial_port.write("9" + "\n" + str(Kd_input) + "\n") # 9 = Just Kd
        print("New Kd sent!")
        
@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == '__main__':
    socketio.run(app, "169.254.74.215", 80)
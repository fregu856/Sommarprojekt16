from flask import Flask, render_template, request, redirect, url_for, session, flash, Response
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
Kp = 0
Kd = 0
AUTO_STATE = 0
manual_state = 0
mode = 0
red_percentage = 0

latest_video_frame = []
cycles_without_web_contact = 0


# dictionary for converting serial incoming data regarding the current manual state:
manual_states = {
    0: "Stop",
    1: "Forward",
    2: "Backward",
    3: "Right",
    4: "Left"
}

# dictionary for converting serial incoming data regarding the current auto state:
auto_states = {
    1: "DEAD_END",
    2: "CORRIDOR",
    3: "OUT_OF_CORRIDOR",
    4: "INTO_JUNCTION",
    5: "DETERMINE_JUNCTION",
    6: "JUNCTION_A_R",
    7: "JUNCTION_A_L",
    8: "JUNCTION_B_R",
    9: "JUNCTION_B_L",
    10: "JUNCTION_C",
    11: "JUNCTION_D",
    12: "OUT_OF_JUNCTION",
    13: 13,
    14: 14,
    15: 15,
    16: 16,
    17: 17,
    18: 18,
    19: 19,
    20: 20
}

# dictionary for converting serial incoming data regarding the current mode:
mode_states = {
    5: "Manual",
    6: "Auto"
}

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
    # enable the thread to modify the global variable 'latest_video_frame': (this variable will be accessed by functions doing some sort of video analysis or video streaming)
    global latest_video_frame   
    
    # create an instance of the RPI camera class:
    camera = PiCamera() 
    
    # rotate the camera view 180 deg (I have the RPI camera mounted upside down):
    camera.hflip = True
    camera.vflip = True 
    
    # set resolution and frame rate:
    camera.resolution = (640, 480)
    camera.framerate = 30
    
    # create a generator 'video_frame_generator' which will continuously capture video frames 
    # from the camera and save them one by one in the container 'generator_output': ('video_frame_generator' is an infinte iterator which on every iteration (every time 'next()' is called on it, like eg in a for loop) gets a video frame from the camera and saves it in 'generator_output'))  
    generator_output = PiRGBArray(camera, size=(640, 480))
    video_frame_generator = camera.capture_continuous(generator_output, format="bgr", use_video_port=True)
    
    # allow the camera to warmup:
    time.sleep(0.1)
    
    for item in video_frame_generator:
        # get the numpy array representing the latest captured video frame from the camera
        # and save it globally for everyone to access:
        latest_video_frame = generator_output.array 
        
        # clear the output container so it can store the next frame in the next loop cycle:
        # (please note that this is absolutely necessary!)
        generator_output.truncate(0)        
        
        # delay for 0.033 sec (for ~ 30 Hz loop frequency):
        time.sleep(0.033) 
        
def stop_runaway_robot():
    # set mode to manual and manual_state to stop: (and everything else to the maximum number for their data type to mark that they are not to be read)
    start_byte = np.uint8(100)
    manual_state = np.uint8(0)
    mode = np.uint8(5)
    Kp = np.uint8(0xFF)
    Kd = np.uint16(0xFFFF)
    Kd_low = np.uint8((Kd & 0x00FF))
    Kd_high = np.uint8((Kd & 0xFF00)/256)
    
    # caculate checksum for the data bytes to be sent:
    checksum = np.uint8(manual_state + mode + Kp + Kd_low + Kd_high)
    
    # send all data bytes:
    serial_port.write(start_byte.tobytes())
    serial_port.write(manual_state.tobytes())
    serial_port.write(mode.tobytes())
    serial_port.write(Kp.tobytes())
    serial_port.write(Kd_low.tobytes())
    serial_port.write(Kd_high.tobytes())
    
    # send checksum:
    serial_port.write(checksum.tobytes())

def web_thread():
    global cycles_without_web_contact
    
    while 1:
        # send all data for display on the web page:
        socketio.emit("new_data", {"IR_0": IR_0, "IR_1": IR_1, "IR_2": IR_2, "IR_3": IR_3, "IR_4": IR_4, "IR_Yaw_right": IR_Yaw_right, "IR_Yaw_left": IR_Yaw_left, "Yaw": Yaw, "p_part": p_part, "alpha": alpha, "Kp": Kp, "Kd": Kd, "AUTO_STATE": AUTO_STATE, "manual_state": manual_state, "mode": mode, "red_percentage": red_percentage})
        cycles_without_web_contact += 1
        if cycles_without_web_contact > 5: # if we havn't had wifi contact for ~ 0.5 sec: stop the robot!
            stop_runaway_robot()
        # delay for 0.1 sec (for ~ 10 Hz loop frequency):
        time.sleep(0.1) 
        
def read_serial_thread():
    # all global variables this function can modify:
    global IR_0, IR_1, IR_2, IR_3, IR_4, IR_Yaw_right, IR_Yaw_left, Yaw, p_part, alpha, Kp, Kd, AUTO_STATE, manual_state, mode, red_percentage

    while 1:
        no_of_bytes_waiting = serial_port.inWaiting()
        if no_of_bytes_waiting > 19: # the ardu sends 20 bytes at the time (18 data, 2 control)
            # read the first byte (read 1 byte): (ord: gives the actual value of the byte)
            first_byte = np.uint8(ord(serial_port.read(size = 1))) 
            
            # read all data bytes if first byte was the start byte:
            if first_byte == 100:
                serial_data = []
                # read all data bytes:
                for counter in range(18): # 18 data bytes is sent from the ardu
                    serial_data.append(ord(serial_port.read(size = 1)))
                
                # read the received checksum:
                checksum = np.uint8(ord(serial_port.read(size = 1)))
                
                # calculate checksum for the received data bytes: (pleae note that the use of uint8 and int8 exactly match what is sent from the arduino)
                calc_checksum = np.uint8(np.uint8(serial_data[0]) + np.uint8(serial_data[1]) + 
                    np.uint8(serial_data[2]) + np.uint8(serial_data[3]) + np.uint8(serial_data[4]) + 
                    np.int8(serial_data[5]) + np.int8(serial_data[6]) + np.int8(serial_data[7]) + 
                    np.int8(serial_data[8]) + np.int8(serial_data[9]) + np.int8(serial_data[10]) + 
                    np.uint8(serial_data[11]) + np.uint8(serial_data[12]) + np.uint8(serial_data[13]) + 
                    np.uint8(serial_data[14]) + np.uint8(serial_data[15]) + np.uint8(serial_data[16]) + np.uint8(serial_data[17]))

                # update the variables with the read serial data only if the checksums match:
                if calc_checksum == checksum:
                    IR_0 = int(np.uint8(serial_data[0])) # (int() is needed to convert it something that can be sent to the webpage) 
                    IR_1 = int(np.uint8(serial_data[1]))
                    IR_2 = int(np.uint8(serial_data[2]))
                    IR_3 = int(np.uint8(serial_data[3]))
                    IR_4 = int(np.uint8(serial_data[4]))
                    IR_Yaw_right = int(np.int8(serial_data[5])) # IR_Yaw_right was sent as an int8_t, but in order to make python treat it as one we first need to tell it so explicitly with the help of numpy, before converting the result (a number between -128 and +127) to the corresponding python int 
                    IR_Yaw_left = int(np.int8(serial_data[6]))
                    Yaw = int(np.int8(serial_data[7]))
                    p_part = int(np.int8(serial_data[8]))
                    alpha_low_byte = np.uint8(serial_data[9])
                    alpha_high_byte = np.uint8(serial_data[10]) # yes, we need to first treat both the low and high bytes as uint8:s, try it by hand and a simple example (try sending -1)
                    alpha = int(np.int16(alpha_low_byte + alpha_high_byte*256)) # (mult with 256 corresponds to a 8 bit left shift)
                    Kp = int(np.uint8(serial_data[11]))
                    Kd_low_byte = np.uint8(serial_data[12])
                    Kd_high_byte = np.uint8(serial_data[13])
                    Kd = int(Kd_low_byte + Kd_high_byte*256)
                    AUTO_STATE = auto_states[int(np.uint8(serial_data[14]))] # look up the received integer in the auto_states dict
                    manual_state = manual_states[int(np.uint8(serial_data[15]))]
                    mode = mode_states[int(np.uint8(serial_data[16]))]
                    red_percentage = int(np.uint8(serial_data[17]))
                else: # if the checksums doesn't match: something weird has happened during transmission: flush input buffer and start over
                    serial_port.flushInput()
                    print("Something went wrong in the transaction: checksums didn't match!") 
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")   
                    print("Something went wrong in the transaction: checksums didn't match!")                       
            else: # if first byte isn't the start byte: we're not in sync: just read the next byte until we get in sync (until we reach the start byte)
                pass
        else: # if not enough bytes for entire transmission, just wait for more data:
            pass

        time.sleep(0.025) # Delay for ~40 Hz loop frequency (faster than the sending frequency)

def gen_normal():
    while 1:
        if len(latest_video_frame) > 0: # if we have started receiving actual frames:
            # convert the latest read video frame to jpg format:
            ret, jpg = cv2.imencode(".jpg", latest_video_frame) 
            
            # get the raw data bytes of the jpg image: (convert to binary)
            frame = jpg.tobytes()
            
            # yield ('return') the frame: (yield: returns value and saves the current state of the generator function, the next time this generator function is called execution will resume on the next line of code in the function (ie it will in this example start a new cycle of the while loop and yield a new frame))
            # what we yield looks like this, but in binary: (binary data is a must for multipart)
            # --frame
            # Content-Type: image/jpeg
            #
            # <frame data>
            #
            yield (b'--frame\nContent-Type: image/jpeg\n\n' + frame + b'\n')
                
def gen_mask():
    while 1:
        if len(latest_video_frame) > 0: # if we have started receiving actual frames:
            # convert the latest read video frame to HSV (Hue, Saturation, Value) format:
            hsv = cv2.cvtColor(latest_video_frame, cv2.COLOR_BGR2HSV)
            
            # specify lower and upper 'redness' filter boundries:
            lower_blue = np.array([100, 50, 50]) # = [H-20, 100, 100]
            upper_blue = np.array([140, 255, 255]) # = [H+20, 100, 100]
            
            lower_yellow = np.array([25, 100, 100]) # = [H-20, 100, 100]
            upper_yellow = np.array([35, 255, 255]) # = [H+20, 100, 100]
            
            # mask the image according to the 'redness' filter: (pixels which are IN the 'redness' range specified above will be made white, pixels which are outside the 'redness' range (which aren't red enough) will be made black)
            range_mask = cv2.inRange(hsv, lower_blue, upper_blue)
 #           range_mask = cv2.inRange(hsv, lower_yellow, upper_yellow)
               
            no_of_red_pixels = np.nonzero(range_mask)[0].size # np.nonzero(range_mask) is in this case a tuple where the first element is an array containing all row/column indices of the non-zero elements (pixels), and the second is an array containing all column/row indices. Number of non-zero elements (pixels) thus = size of the first element = size of the second element
            red_percentage = int( np.float( (np.float(no_of_red_pixels) / np.float(640*480))*100 ) )

            
            
            # get manual state, set start byte and set everything else to the maximum number for their data type to mark that they are not to be read:
            start_byte = np.uint8(100)
            manual_state = np.uint8(0xFF)
            mode = np.uint8(0xFF)
            Kp = np.uint8(0xFF)
            Kd = np.uint16(0xFFFF)
            Kd_low = np.uint8((Kd & 0x00FF))
            Kd_high = np.uint8((Kd & 0xFF00)/256)
            red_percentage = np.uint8(red_percentage)
            
            # caculate checksum for the data bytes to be sent:
            checksum = np.uint8(manual_state + mode + Kp + Kd_low + Kd_high + red_percentage)
            
            # send all data bytes:
            serial_port.write(start_byte.tobytes())
            serial_port.write(manual_state.tobytes())
            serial_port.write(mode.tobytes())
            serial_port.write(Kp.tobytes())
            serial_port.write(Kd_low.tobytes())
            serial_port.write(Kd_high.tobytes())
            serial_port.write(red_percentage.tobytes())
            
            # send checksum:
            serial_port.write(checksum.tobytes())
            
            
            
            # convert the result to jpg format:
            ret, jpg = cv2.imencode(".jpg", range_mask)
            
            # get the raw data bytes of the jpg image: (convert to binary)
            frame = jpg.tobytes()
            
            # yield the frame:
            # what we yield looks like this, but in binary: (binary data is amust for multipart)
            # --frame
            # Content-Type: image/jpeg
            #
            # <frame data>
            #
            yield (b'--frame\nContent-Type: image/jpeg\n\n' + frame + b'\n')
            time.sleep(0.1)
                
@app.route("/camera_normal")
def camera_normal():
    # return a Respone object with a 'gen_normal()' generator function as its data generating iterator. We send a MIME multipart message of subtype Mixed-replace, which means that the browser will read data parts (generated by gen_obj_normal) one by one and immediately replace the previous one and display it. We never close the connection to the client, pretending like we haven't finished sending all the data, and constantly keeps sending new data parts generated by gen_obj_normal.
    # what over time will be sent to the client is the following:
    # Content-Type: multipart/x-mixed-replace; boundary=frame
    #
    # --frame
    # Content-Type: image/jpeg
    #
    #<jpg data>
    #
    # --frame
    # Content-Type: image/jpeg
    #
    #<jpg data>
    #
    # etc, etc
    # where each '--frame' enclosed section represents a jpg image taken from the camera that the browser will read and display one by one, replacing the previous one, thus generating a video stream
    gen_obj_normal = gen_normal()
    return Response(gen_obj_normal, mimetype = "multipart/x-mixed-replace; boundary=frame")
    
@app.route("/camera_mask")
def camera_mask():
    # return a Respone object with a 'gen_mask()' generator function as its data generating iterable, se "camera_normal"
    gen_obj_mask = gen_mask()
    return Response(gen_obj_mask, mimetype = "multipart/x-mixed-replace; boundary=frame")
    
@app.route("/")   
@app.route("/index")
def index():
    try:
        # start a thread constantly reading frames from the camera:
        thread_video = Thread(target = video_thread)
        thread_video.start()
        
        # start a thread constantly sending sensor/status data to the web page: 
        thread_web = Thread(target = web_thread)
        thread_web.start()
        
        # start a thread constantly reading sensor/status data from the arduino:
        thread_read_serial = Thread(target = read_serial_thread)
        thread_read_serial.start()
        
        return render_template("index.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
          
@app.route("/phone")
def phone():
    try:
        # start a thread constantly reading frames from the camera:
        thread_video = Thread(target = video_thread)
        thread_video.start()
        
        # start a thread constantly sending sensor/status data to the web page: 
        thread_web = Thread(target = web_thread)
        thread_web.start()
        
        # start a thread constantly reading sensor/status data from the arduino:
        thread_read_serial = Thread(target = read_serial_thread)
        thread_read_serial.start()
        
        return render_template("phone.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))        
        
@socketio.on("my event")
def handle_my_custom_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
       
@socketio.on("arrow_event")
def handle_arrow_event(sent_dict):
    print("Recieved message: " + str(sent_dict["data"]))
    
    # get manual state, set start byte and set everything else to the maximum number for their data type to mark that they are not to be read:
    start_byte = np.uint8(100)
    manual_state = np.uint8(sent_dict["data"])
    mode = np.uint8(0xFF)
    Kp = np.uint8(0xFF)
    Kd = np.uint16(0xFFFF)
    Kd_low = np.uint8((Kd & 0x00FF))
    Kd_high = np.uint8((Kd & 0xFF00)/256)
    red_percentage = np.uint8(0xFF)
    
    # caculate checksum for the data bytes to be sent:
    checksum = np.uint8(manual_state + mode + Kp + Kd_low + Kd_high + red_percentage)
    
    # send all data bytes:
    serial_port.write(start_byte.tobytes())
    serial_port.write(manual_state.tobytes())
    serial_port.write(mode.tobytes())
    serial_port.write(Kp.tobytes())
    serial_port.write(Kd_low.tobytes())
    serial_port.write(Kd_high.tobytes())
    serial_port.write(red_percentage.tobytes())
    
    # send checksum:
    serial_port.write(checksum.tobytes())
    
@socketio.on("mode_event")
def handle_mode_event(sent_dict):
    print("Recieved message: " + str(sent_dict["data"]))
    
    # get mode state, set start byte and set everything else to the maximum number for their data type to mark that they are not to be read:
    start_byte = np.uint8(100)
    manual_state = np.uint8(0xFF)
    mode = np.uint8(sent_dict["data"])
    Kp = np.uint8(0xFF)
    Kd = np.uint16(0xFFFF)
    Kd_low = np.uint8((Kd & 0x00FF))
    Kd_high = np.uint8((Kd & 0xFF00)/256)
    red_percentage = np.uint8(0xFF)
    
    # caculate checksum for the data bytes to be sent:
    checksum = np.uint8(manual_state + mode + Kp + Kd_low + Kd_high + red_percentage)
    
    # send all data bytes:
    serial_port.write(start_byte.tobytes())
    serial_port.write(manual_state.tobytes())
    serial_port.write(mode.tobytes())
    serial_port.write(Kp.tobytes())
    serial_port.write(Kd_low.tobytes())
    serial_port.write(Kd_high.tobytes())
    serial_port.write(red_percentage.tobytes())
    
    # send checksum:
    serial_port.write(checksum.tobytes())
    
@socketio.on("parameters_event")
def handle_parameters_event(sent_dict):
    Kp_input = sent_dict["Kp"]
    Kd_input = sent_dict["Kd"]   
    print("Recieved Kp: " + Kp_input)
    print("Recieved Kd: " + Kd_input)
    
    # check parameter input and set Kp, Kd to the parameter input only it it is non-empty and valid:
    Kp_input = check_parameter_input(Kp_input) # After this, Kp_input is non-empty only if the user typed a positive (>= 0) integer into the Kp field (and thus we should send it to the arduino)
    Kd_input = check_parameter_input(Kd_input)      
    if (Kp_input or Kp_input == 0) and (Kd_input or Kd_input == 0): # if valid, non-empty input for both Kp and Kd: send both Kp and Kd
        Kp = np.uint8(Kp_input)
        Kd = np.uint16(Kd_input)
    elif Kp_input or Kp_input == 0: # if only Kp:
        Kp = np.uint8(Kp_input)
        Kd = np.uint16(0xFFFF)
    elif Kd_input or Kd_input == 0: # if only Kd:
        Kp = np.uint8(0xFF)
        Kd = np.uint16(Kd_input)
    else: # if neither is valid/non-empty:
        Kp = np.uint8(0xFF)
        Kd = np.uint16(0xFFFF)
    Kd_low = np.uint8((Kd & 0x00FF))
    Kd_high = np.uint8((Kd & 0xFF00)/256)
    
    # set start byte and set everything else to the maximum number for their data type to mark that they are not to be read:
    start_byte = np.uint8(100)
    manual_state = np.uint8(0xFF)
    mode = np.uint8(0xFF)
    red_percentage = np.uint8(0xFF)
    
    # caculate checksum for the data bytes to be sent:
    checksum = np.uint8(manual_state + mode + Kp + Kd_low + Kd_high + red_percentage)
    
    # send all data bytes:
    serial_port.write(start_byte.tobytes())
    serial_port.write(manual_state.tobytes())
    serial_port.write(mode.tobytes())
    serial_port.write(Kp.tobytes())
    serial_port.write(Kd_low.tobytes())
    serial_port.write(Kd_high.tobytes())
    serial_port.write(red_percentage.tobytes())
    
    # send checksum:
    serial_port.write(checksum.tobytes())
    
@socketio.on("control_event")
def handle_control_event(sent_dict):
    global cycles_without_web_contact
    cycles_without_web_contact = 0
        
@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == '__main__':
    socketio.run(app, "169.254.74.215", 80)
from flask import Flask, render_template, request, redirect, url_for, session, flash
from dbconnect import connect
import gc   # Garbage collection
from datetime import date
import numbers
from flask_socketio import SocketIO
import serial

#stop = 0
#forward = 1
#backward = 2
#right = 3
#left = 4

# 9600 is the baudrate, should match serial baudrate in arduino
serial_port = serial.Serial('/dev/ttyACM0', 9600) 

app = Flask(__name__)
socketio = SocketIO(app)

@app.route("/")   
@app.route("/index")
def index():
    try:
        return render_template("index.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
          
@app.route("/phone")
def phone():
    try:
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

@socketio.on("touch_event")
def handle_touch_event(sent_dict):
    print("Recieved message: " + sent_dict["data"])
    serial_port.write(sent_dict["data"])       

@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == '__main__':
    socketio.run(app, "169.254.74.215", 80)
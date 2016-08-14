# Sommarprojekt16 (Summer project 16)

Personal summer project.

Video:
https://youtu.be/povGte7K8o4

Pictures from the build:
https://goo.gl/photos/z78dbTpEZ5kwjqneA

Main hardware:
1x Arduino Uno
1x Raspberry Pi 3 Model B
1x Raspberry Pi Camera Module v2
1x DFRobot 4WD Arduino Mobile Platform (http://www.robotshop.com/eu/en/dfrobot-4wd-arduino-mobile-platform.html)
2x L298 Dual H-Bridge DC Motor Controller (http://www.robotshop.com/eu/en/l298-dual-h-bridge-dc-motor-controller.html)
5x Sharp GP2Y0A21YK0F IR Range Sensor - 10cm to 80cm (http://www.robotshop.com/eu/en/sharp-gp2y0a21yk0f-ir-range-sensor.html)

Things/links that I have used or found useful (in no particular order):

Flask (back-end python):
http://flask.pocoo.org/

Flask-SocketIO (back-end <--> front-end):
https://flask-socketio.readthedocs.io/en/latest/
http://blog.miguelgrinberg.com/post/easy-websockets-with-flask-and-gevent

SocketIO (front-end <--> back-end):
http://socket.io/docs/

eventlet (webserver for Flask-SocketIO):
http://eventlet.net/

Numpy:
http://docs.scipy.org/doc/

pySerial:
http://pyserial.readthedocs.io/en/latest/pyserial_api.html

picamera (the RPI camera in python):
http://picamera.readthedocs.io/en/release-1.12/

OpenCV:
http://opencv.org/

Bootstrap (front-end prettifier):
http://getbootstrap.com/

Keen dashboards:
https://keen.github.io/dashboards/layouts/

CanvasJs (live updating graphs):
http://canvasjs.com/editor/?id=http://canvasjs.com/example/gallery/dynamic/realtime_line/

How to setup the RPI as a WiFi hotspot:
https://frillip.com/using-your-raspberry-pi-3-as-a-wifi-access-point-with-hostapd/

Accessing the RPI camera with OpenCV and python:
http://www.pyimagesearch.com/2015/03/30/accessing-the-raspberry-pi-camera-with-opencv-and-python/

Basic serial communication RPI <--> Arduino:
https://oscarliang.com/connect-raspberry-pi-and-arduino-usb-cable/

Video streaming with Flask:
http://blog.miguelgrinberg.com/post/video-streaming-with-flask

Great explanation of generators and "yield" in python (needed to understand the video streaming):
https://jeffknupp.com/blog/2013/04/07/improve-your-python-yield-and-generators-explained/

Some background info for the video streaming:
https://en.wikipedia.org/wiki/MIME
http://www.howtocreate.co.uk/php/serverpushdemo.php

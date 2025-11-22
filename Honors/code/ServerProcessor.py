#This is a basic script to host a webpage at the IP specified
# By the 'IP_Address' variable

# Import library to create webserver to host webpage

#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  camera_pi.py
#  
#  
#  
import struct
import time
import io
import threading
import picamera
import base64
import cv2
import zmq
from threading import Thread

#for the image processing
import cv2
import numpy as np

from flask import Flask, render_template
from flask import Flask, render_template, Response,redirect,request, url_for
import itertools
# import time
from camera_pi import Camera
import socket


app = Flask(__name__)

#Camera Socket Server Address
#server_address_camera =  ('127.0.0.2', 8001)
#sock_camera = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#sock_camera.bind(server_address_camera)

#C++ File Socket Server Address
server_address_cpp =  ('127.0.0.1', 8000)
sock_cpp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_cpp.bind(server_address_cpp)
sock_cpp.listen(5)
connection, address = sock_cpp.accept()


#Find the IP Address of your device
#Use the 'ifconfig' terminal command, the address should be in the format  "XX.XXX.XXX.XXX"
IP_Address = '10.227.31.206'
PORT = 8080
#Connect the *.html page to the server and run as the default page

class Camera(object):
    thread = None  # background thread that reads frames from camera
    frame = None  # current frame is stored here by background thread
    last_access = 0  # time of last client access to the camera

    def initialize(self):
        if Camera.thread is None:
            # start background frame thread
            Camera.thread = threading.Thread(target=self._thread)
            Camera.thread.start()

            # wait until frames start to be available
            while self.frame is None:
                time.sleep(0)

    def get_frame(self):
        Camera.last_access = time.time()
        self.initialize()
        return self.frame

    @classmethod
    def _thread(cls):
        with picamera.PiCamera() as camera:
            # Camera setup: camera.resolution = (X,X)
            camera.resolution = (320, 240)
            camera.hflip = False
            camera.vflip = False

            stream = io.BytesIO()
            for foo in camera.capture_continuous(stream, 'jpeg',
                                                 use_video_port=True):
                # store frame
                stream.seek(0)
                cls.frame = stream.read()

                # reset stream for next frame
                stream.seek(0)
                stream.truncate()

                # if there hasn't been any clients asking for frames in
                # the last 10 seconds stop the thread
                ######################################
                if time.time() - cls.last_access > 10:
                    break
        cls.thread = None

camera = Camera()

@app.route('/')
def index():
    if request.headers.get('accept') == 'text/event-stream':
        def events():
            for i, c in enumerate(itertools.cycle('\|/-')):  
                yield "data: %s\n\n" % ('b0c0d0')
        return Response(events(), content_type='text/event-stream')
    return render_template('FinalEXE3.html')


def gen(camera):
    max_len = 65507
    frame = b''
    while True:
        # receive image to the client: frame,_ = .....
        frame = camera.get_frame()
        radius = processFrame(frame)
        yield (b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        
        data = struct.pack('<f', radius)  # convert float to 4 bytes
        try:
            connection.sendall(data)
        except BrokenPipeError:
            print("Client disconnected, stopping send.")

        time.sleep(0.2)


def processFrame(inputFrame):
    # Convert to OpenCV image
    img_np = np.frombuffer(inputFrame, dtype=np.uint8)
    img = cv2.imdecode(img_np, cv2.IMREAD_COLOR)

    # Convert to grayscale and blur
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5,5), 0)

    # Threshold for black line
    _, thresh = cv2.threshold(blur, 100, 255, cv2.THRESH_BINARY_INV)

    h, w = thresh.shape

    # Bottom 50% ROI
    roi = thresh[int(h*0.2):int(h*0.7), :]

    # Contours
    results = cv2.findContours(roi, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if len(results) == 3:
        _, contours, _ = results
    else:
        contours, _ = results

    if len(contours) == 0:
        print("No contours")
        return 0

    # Largest contour = line
    cnt = max(contours, key=cv2.contourArea)

    M = cv2.moments(cnt)
    if M["m00"] == 0:
        print("Zero moment")
        return 0

    cx = int(M["m10"] / M["m00"])

    # --- CORRECT WIDTH for centroid error ---
    roi_h, roi_w = roi.shape
    error_x = cx/(roi_w // 2) - 1

    print(cx, error_x)

    # --- Linear radius model ---
    R = error_x

    #print("cx:", cx, "norm_error:", norm_error, "R:", R, "angular_speed:", angular_speed)
    #print(R)
    return R

@app.route('/video_feed')
def video_feed():
    return Response(gen(Camera()),mimetype='multipart/x-mixed-replace; boundary=frame')


#Start the server
if __name__ == "__main__":
    app.run(debug=True, host=IP_Address, port=PORT, use_reloader=False)
    #read_send_image()

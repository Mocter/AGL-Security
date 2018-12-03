# Server: ------------------------------------------------------------
# Description: web server used to host AGL security website built on flask
# Author: Luke Marche
# Modified by: none
# Date: Dec 1st 2018
#-----------------------------------------------------------------------------

# Header Files-----------
from flask import Flask, render_template
import serial
app = Flask(__name__)

import time
#------------------------

#sleep for 10 seconds to get an ip from the DHCP server 
time.sleep(10)

# Global Variables-------
statusDict = {
    "floodStatus" : "",
    "smokeStatus" : "",
    "doorStatus" : "",
    "motionStatus" : "",
    "armedStatus" : ""
}
statusMessage = []
#------------------------

# create and initialize the serial port object on the raspberry pi
ser = serial.Serial(
        port = '/dev/serial0',
        baudrate= 9600,
        parity = serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
)


# readserialport:****************************************************************
# Function Name: readserialport
# Parameters: none
# Return: None
# Description: read a message from the serial port and set the values in the sensor status dictionary
# Author: Luke Marche
# Modified by: none
# ****************************************************************************
def readSerialPort():
    if ser.in_waiting > 0:
        statusMessage = ser.readline()
        ser.reset_input_buffer()
        print statusMessage
        if statusMessage[0] == "1":
            statusDict["floodStatus"] = "ALARM"
        else:
            statusDict["floodStatus"] = "Normal"

        if statusMessage[1] == "1":
            statusDict["smokeStatus"] = "ALARM"
        else:
            statusDict["smokeStatus"] = "Normal"

        if statusMessage[2] == "1":
            statusDict["doorStatus"] = "ALARM"
        else:
            statusDict["doorStatus"] = "Normal"

        if statusMessage[3] == "1":
            statusDict["motionStatus"] = "ALARM"
        else:
            statusDict["motionStatus"] = "Normal"

        if statusMessage[4] == "1":
            statusDict["armedStatus"] = "Armed"
        else:
            statusDict["armedStatus"] = "Unarmed"
    





#on home page refresh
@app.route("/")
# home:****************************************************************
# Function Name: home
# Parameters: none
# Return: None
# Description: display the index.html template and pass the dictionary values to their place holders the html page
# Author: Luke Marche
# Modified by: none
def home():
    readSerialPort()
    return render_template('index.html', smoke = statusDict["smokeStatus"], flood = statusDict["floodStatus"], door = statusDict["doorStatus"], motion = statusDict["motionStatus"], armed = statusDict["armedStatus"])

#run the server at the specified ip address and port 
if __name__ == '__main__':
	app.run(host = '192.168.50.74', port=3000)



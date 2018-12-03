// AGL Security MBED Program: ------------------------------------------------------------
// Description: Main Controller for Security System
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
//----------------------------------------------------------------------------------------

// Header Files-----------
#include "mbed.h"
#include "TextLCD.h"
#include "Dht11.h"
#include "MQ2.h"
#include "ID12RFID.h"
//------------------------

// Define Statements------
#define ARMED          1
#define UNARMED        0
#define MESSAGESIZE    5
//Defines for Sensors
#define HIGHSMOKELEVEL 1000
#define DOOROPEN       0
#define MOTIONSENSED   1
#define HIGHHUMDITY    60
//Defines for Status Message Array
#define HUMPOSN        0
#define SMOKEPOSN      1
#define DOORPOSN       2
#define MOTIONPOSN     3
#define ARMEDPOSN      4
//------------------------

// Global Varibles-------

//Lcd Screen initalization
TextLCD lcd(p15, p16, p17, p18, p19, p21); // rs, e, d4-d7
 
//Contact Sensor initalization
DigitalIn ContactSensor(p28);

//Gas Sensor initalization
MQ2 mq2(p20);        
MQ2_data_t MQ2_data;

//Humidity Sensor initalization
Dht11 HumSensor(p27);
 
//Motion Sensor initalization
DigitalIn motionSensor(p26);

//RFID Reader
ID12RFID rfid(p14); // uart rx

//used for debugging
Serial pc(USBTX, USBRX); // tx, rx
//------------------------

// Function Prototypes---
char readRfid(char armed, char messageOut[MESSAGESIZE], int tag1, int tag2);
char readDoorContact(char armed, char messageOut[MESSAGESIZE]);
void readSmokeSensor(char messageOut[MESSAGESIZE]);
void readHumiditySensor(char messageOut[MESSAGESIZE]);
char readMotionSensor(char armed, char messageOut[MESSAGESIZE]);
bool checkMessage(char messageOut[MESSAGESIZE]);
//-----------------------

// Main:****************************************************************
// Function Name: main
// Parameters: None
// Return: None
// Description: intializes serial ports, houses Tag ids, calls all functions in an infinite loop
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
int main() 
{
    //tag ids
    int tag1 = 9804081;
    int tag2 = 9770265;
    //serial port used to send data to raspberry pi 
    Serial dp1(p9, p10);
    //set baud rates
    pc.baud(115200);
    dp1.baud(9600);
    //initialize  gas sensor
    mq2.begin();
    
    char statusMessage[] = "00000\n"; //create status message
    char armed = 0;
    bool isValid;
    while(1) //infinite loop
    {  
        wait(5);
        armed = readRfid(armed, statusMessage, tag1, tag2);
        armed = readDoorContact(armed, statusMessage);
        readSmokeSensor(statusMessage);
        readHumiditySensor(statusMessage);
        armed = readMotionSensor(armed, statusMessage);
        pc.printf(statusMessage); //used for debugging
        isValid = checkMessage(statusMessage);
        //check if there is a valid message to be sent
        if (isValid == true)
        {
        dp1.printf(statusMessage);
        //pc.printf("messageSent\n"); used for debugging
        }
    }      
}//eo: main() ==================================================================

// readDoorContact:*****************************************************
// Function Name: readDoorContact
// Parameters: char armed, char messageOut[MESSAGESIZE]
// Return: armed
// Description: check if the door is open and system is armed, display status on lcd and update the message to the raspberry pi
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
char readDoorContact(char armed, char messageOut[MESSAGESIZE])
{
    if ((ContactSensor == DOOROPEN) && armed == ARMED)
            {  
            lcd.printf("The door was opened.            \n");
            messageOut[DOORPOSN] = '1';
            wait (0.2);
            }
            else
            {
            messageOut[DOORPOSN] = '0';  
            }
            return armed;
}//eo: readDoorContact(char armed, char messageOut[MESSAGESIZE]) ==================================================================


// readSmokeSensor:*****************************************************
// Function Name: readSmokeSensor
// Parameters: char messageOut[MESSAGESIZE]
// Return: nothing
// Description: check if smoke sensor is reading a level higher than the high smoke level, display status on lcd and update the message to the raspberry pi
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
void readSmokeSensor(char messageOut[MESSAGESIZE])
{
    mq2.read(&MQ2_data);
    if (MQ2_data.smoke > HIGHSMOKELEVEL) 
            {
            lcd.printf("Smoke has been detected.        \n");
            wait (0.2);
            messageOut[SMOKEPOSN] = '1';                                            
            }
            else
            {
            messageOut[SMOKEPOSN] = '0';  
            }          
}//eo: readSmokeSensor(char messageOut[MESSAGESIZE]) ==================================================================

// readHumiditySensor:**************************************************
// Function Name: readHumiditySensor
// Parameters: char messageOut[MESSAGESIZE]
// Return: nothing
// Description: check if humidity sensor is reading a level higher than the high humidity level, display status on lcd and update the message to the raspberry pi
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
void readHumiditySensor(char messageOut[MESSAGESIZE])
{
    HumSensor.read();
    if (HumSensor.getHumidity() > HIGHHUMDITY)
            {
            lcd.printf("Water has been detected.        \n");
            wait (0.2); 
            messageOut[HUMPOSN] = '1';
            }
            else
            {
            messageOut[HUMPOSN] = '0'; 
            }
}//eo: readHumiditySensor(char messageOut[MESSAGESIZE]) ==================================================================

// readMotionSensor:****************************************************
// Function Name: readMotionSensor
// Parameters: char armed, char messageOut[MESSAGESIZE]
// Return: armed
// Description: check if the sensor detects motion, display status on lcd and update the message to the raspberry pi
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
char readMotionSensor(char armed, char messageOut[MESSAGESIZE])
{
        if (motionSensor == MOTIONSENSED  && armed == ARMED)
            {        
            lcd.printf("Motion has been detected.       \n");
            messageOut[MOTIONPOSN] = '1';
            wait(0.2);
            }
            else
            {
            messageOut[MOTIONPOSN] = '0';  
            }
            return armed;
}//eo: readMotionSensor(char armed, char messageOut[MESSAGESIZE]) ==================================================================

// readRfid:****************************************************
// Function Name: readRfid
// Parameters: char armed, char messageOut[MESSAGESIZE], int tag1, int tag2
// Return: armed
// Description: read a tag then check if the tag being read is one of the known tags. this changes the status of the armed variable and message sent to the raspberry pi
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
char readRfid(char armed, char messageOut[MESSAGESIZE], int tag1, int tag2)
{
    if(rfid.readable())
            {
              int readTag = rfid.read();
              if((readTag == tag1 || readTag == tag2) && armed == UNARMED)
              {
              pc.printf("system armed");
              armed = ARMED; 
              messageOut[ARMEDPOSN] = '1';
              }
              else if((readTag == tag1 || readTag == tag2) && armed == ARMED)
              {
              pc.printf("system disarmed");  
              armed = UNARMED;
              messageOut[ARMEDPOSN] = '0';
              lcd.cls();
              }
            }
            return armed;
}//eo: readRfid(char armed, char messageOut[MESSAGESIZE], int tag1, int tag2) ==================================================================

// checkMessage:****************************************************
// Function Name: checkMessage
// Parameters: char messageOut[MESSAGESIZE]
// Return: true/false
// Description: check if each part of the message string is one of the valid values then return true or false
// Authors: Abed Moussa, Greg Loucks, Luke Marche
// Date: Dec 1st 2018
// *********************************************************************
bool checkMessage(char messageOut[MESSAGESIZE])
{   
    for (int i = 0; i < strlen(messageOut)-1; i++)
    {
        if((messageOut[i] == '1') || (messageOut[i] == '0'))
        {
        //pc.printf("val"); used for debugging
        }
        else
        {
        //pc.printf("inv"); used for debugging
        return false;
        }
    }
    return true;
}//eo: checkMessage(char messageOut[MESSAGESIZE]) ==================================================================

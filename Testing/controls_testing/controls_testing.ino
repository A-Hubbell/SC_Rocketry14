//Testing code, adapted from "pseudo_code.ino" with the custom functions pasted into the same file.



//Libraries

#include "Rocket_Sensors.h"
#include "Rocket_Data_Handling.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

//Adafruit GPS objects and settings (MOVE TO SENSOR HEADER FILE??)
Adafruit_GPS GPS(&Serial);
#define GPSECHO  false   //Echo to serial port
boolean usingInterrupt = false;   //Manual interrupt to call data
void useInterrupt(boolean);

//Timer variable to be used throughout program  (MAKE SURE NOT SAME AS GPS TIMER, IF BEING USED)
uint32_t timer = millis();

//Global variables and constants

float dofData[9] = {0,0,0,0,0,0,0,0,0}; //9 channels
float gpsData[10] = {0,0,0,0,0,0,0,0,0,0}; // CHANGE to suit number of required data fields 
float bmpData[4] = {0,0,0,0};  // Altitude  Temperature  Pressure
float vertAccel = 0;

const int LED_PIN = 0;
const int LAUNCH_THRESH = 300; //300 ft altitude detected, switch from mode 1 to mode 2
const int DROGUE_EMATCH_PIN = 1; //Pin connected to drogue parachute e-match
const int MAIN_EMATCH_PIN = 2; //Pin connected to main parachute e-match
const int APOGEE_ACCEL_THRESH = 3; // Acceleration upper threshold for apogee detection
const int MAIN_ALT_THRESH = 1800; //Altitude lower threshold for main chute deployment (1800 ft.)




void setup()
{


//-----------------------------------------------------------------------------------------------------
//PIN INITIALIZATIONS

  pinMode(LED_PIN, OUTPUT);
  pinMode(DROGUE_EMATCH_PIN, OUTPUT);
  pinMode(MAIN_EMATCH_PIN, OUTPUT);
  

//-----------------------------------------------------------------------------------------------------  
//SERIAL PORT INITIALIZATIONS 
  
//Xbee ("Serial2") and SD Logger ("Serial1") communicate at 9600 baud rate, while the GPS ("Serial") uses 115200
  Serial.begin(115200); //GPS
  Serial1.begin(9600); //SD Logger
  Serial2.begin(9600); //Xbee  

//-----------------------------------------------------------------------------------------------------
//GPS INITIALIZATION

setupGPS();

//GPS.begin(9600);
//GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //Time/location data only
//// GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  //Fix data and altitude 
//GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
//useInterrupt(true);
//delay(1000);

//----------------------------------------------------------------------------------------------------- 
//BMP180 PRESSURE SENSOR INTIALIZATION
  
  Serial.println("BMP180 Pressure Sensor Test");
  Serial.println("")
  
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

//-----------------------------------------------------------------------------------------------------

}

//This is from the GPS_testing code that works with Mega2560
SIGNAL(TIMER0_COMPA_vect)
{
  char c = GPS.read();
}


void loop()
{
 
  
 delay(500) // intial delay for all electronics
 

 
 //----------------------------------------------------------------------------------------------------- 
 //Rocket pre-launch algorithm
  do
  {
    //Read 9-DOF data
    getBMP(bmpData[]); //Read BMP180 data
    getGPS(gpsData[]); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  }
  while(bmpData[0] < LAUNCH_THRESH) 
 
 
 
  //----------------------------------------------------------------------------------------------------- 
  //Drogue parachute deployment algorithm 
  //Read and transmit data from all sensors while vertAccel greater than APOGEE_ACCEL_THRESH
  //Send current HIGH to DROGUE parachute e-match when vertAccel less than or equal to APOGEE_ACCEL_THRESH
  do
  {
    //Read 9-DOF data
    getBMP(bmpData[]); //Read BMP180 data
    getGPS(gpsData[]); //Read GPS data
    dataSDOut();  //Transmit data to SD logger, then Xbee
    dataXbeeout();
  }
  while(vertAccel > APOGEE_ACCEL_THRESH)
 
 
  // Set drogue chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee
  digitalWrite(DROGUE_EMATCH_PIN, HIGH);
  
  Serial1.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);
  
  
  //----------------------------------------------------------------------------------------------------- 
  //Main parachute deployment algorithm 
  //Read and transmit data from all sensors while bmpData[0] (rocket altitude) above MAIN_ALT_THRESH altitude
  //Send current HIGH to MAIN parachute e-match when bmpData[0] (rocket altitude) equal to or below MAIN_ALT_THRESH
  do
  {
    //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPS(); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  }
  while(bmpData[0] > MAIN_ALT_THRESH)
  
  
  //Set main chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee  
  digitalWrite(MAIN_EMATCH_PIN, HIGH);

  Serial1.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);


  //----------------------------------------------------------------------------------------------------- 
  //Continue logging all sensor data until turned off
 
  while(true)
  {
    //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPS(); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  } 
 

 
}




//----------------------------------------------------------------------------------------------------- 
//----------------------------------------------------------------------------------------------------- 
//----------------------------------------------------------------------------------------------------- 
//CUSTOM FUNCTIONS HERE!!!



//----------------------------------------------------------------------------------------------------- 
//Data handling functions using


//void dataXbeeOut()
//{
//  
//  //Send all 9-DOF data, 9 channels
//  for(int i = 0; i<9; i++)
//  {
//  Serial2.print(dofData[i]
//  Serial2.print("    ");
//  }
//  
//  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
//  Serial2.println(" ");
//  
//  //Send all BMP180 data, 4 channels
//  for(int i = 0; i<4; i++)
//  {
//  Serial2.print(bmpData[i]);
//  Serial2.print("    ");
//  }
//  
//  
//  Serial2.println(" ");
//  
//  //Send all GPS data, 10 channels
//  for(int i =0; i<10; i++)
//  {
//  Serial2.print(gpsData[i]);
//  Serial2.print("    ");
//  }
//  
//
//  Serial2.println(" ");
//  
//}







//void dataSDOut()
//{
//  
//  //Send all 9-DOF data, 9 channels
//  for(int i = 0; i<9; i++)
//  {
//  Serial1.print(dofData[i]
//  Serial1.print("    ");
//  }
//  
//  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
//  Serial1.println(" ");
//  
//  //Send all BMP180 data, 4 channels
//  for(int i = 0; i<4; i++)
//  {
//  Serial1.print(bmpData[i]);
//  Serial1.print("    ");
//  }
//  
//  
//  Serial1.println(" ");
//  
//  //Send all GPS data, 10 channels
//  for(int i =0; i<10; i++)
//  {
//  Serial1.print(gpsData[i]);
//  Serial1.print("    ");
//  }
//  
//
//  Serial1.println(" ");
//  
//}






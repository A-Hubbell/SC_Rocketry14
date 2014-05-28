/*
Testing code, adapted from "pseudo_code.ino" with the custom functions pasted into the same file.

This is code to be run on the Arduino Mega in the controls bay. It reads serial data from the xbee

Connections:

BMP180 Pressure Sensor SDA --> Mega SDA (pin 20)
BMP180 Pressure Sensor SCL --> Mega SCL (pin 21)

9-DOF Sensor Stick SDA --> Mega SDA (pin 20)
9-DOF Sensor Stick SCL --> Mega SCL (pin 21)

OpenLog SD Logger RX --> Mega TX1 (pin 18)
OpenLog SD Logger TX --> Mega RX1 (pin 19)

Xbee RX --> Mega TX2 (pin 16)
Xbee TX --> Mega RX2 (pin 17)


IMPORTANT:

The "parachuteDeploy" function expects an input parameter of an integer. This is to specify whether the function should
check the current altitude against the requirements for the DROGUE parachute or the MAIN parachute.
The following convention is used:

DROGUE --> 1
MAIN  --> 2

To check if the rocket is at the correct height for DROGUE:   
if (parachuteDeploy(1))
{
 digitalWrite(DROGUE_EMATCH_PIN, HIGH);
}


To check if the rocket is at the correct altitude for MAIN:
if (parachuteDeploy(2))
{
  digitalWrite(MAIN_EMATCH_PIN, HIGH);
}





*/



//Libraries

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_HMC5883_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(00001);

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(00002);

//Adafruit GPS objects and settings (MOVE TO SENSOR HEADER FILE??)
//Adafruit_GPS GPS(&Serial);
//#define GPSECHO  false   //Echo to serial port
//boolean usingInterrupt = false;   //Manual interrupt to call data
//void useInterrupt(boolean);

//Timer variable to be used throughout program  (MAKE SURE NOT SAME AS GPS TIMER, IF BEING USED)
//uint32_t timer = millis();

//Global variables and constants

const int ALTITUDE_BUFFER_SIZE = 10;

float dofData[9] = {0,0,0,0,0,0,0}; //9 channels (accel xyz, mag xyz, heading, 2 unused)
float gpsData[8] = {0,0,0,0,0,0,0,0}; // CHANGE to suit number of required data fields 
float bmpData[4] = {0,0,0,0};  // Altitude  Temperature  Pressure
float altitudeBuffer[ALTITUDE_BUFFER_SIZE]; //Variable to hold consecutive altitude readings for averaging
unsigned long bufferMarker = 0;
float vertAccel = 0;

const int LED_PIN = 0;
const int LAUNCH_THRESH = 300; //300 ft altitude detected, switch from mode 1 to mode 2  CHECK WITH HARLEY, METRES OR FEET??
const int DROGUE_EMATCH_PIN = 2; //Pin connected to drogue parachute e-match
const int MAIN_EMATCH_PIN = 3; //Pin connected to main parachute e-match
const int APOGEE_ACCEL_THRESH = 3; // Acceleration upper threshold for apogee detection
const int MAIN_ALT_THRESH = 1800; //Altitude lower threshold for main chute deployment (1800 ft.)
const char PRESSURE_CODE[10] = "pressure:"; //String for determining if sent data is request to change sea level pressure
const char PARACHUTE_CODE[10] = "LaunchPar";  // String for determining of sent data is request to manually deploy parachute


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);



uint32_t timer = millis();

float seaLevelPressure = 0.0;

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
  Serial1.begin(9600); //SD Logger
  Serial2.begin(9600); //Xbee  

//----------------------------------------------------------------------------------------------------- 
//BMP180 PRESSURE SENSOR INTIALIZATION
  
  Serial.println("BMP180 Pressure Sensor Test");
  Serial.println("");
  
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Oops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

//-----------------------------------------------------------------------------------------------------

}


void loop()
{
 
  
 delay(500); // intial delay for all electronics
 
 if (Serial.available() > 0 ) // Check for incoming serial data
  {
    char dataRecieved[100] = {'\0'};
    int arraySize = 0;
    while (Serial.available() > 0) { // While we have serial data read it into an array of characters
        dataRecieved[arraySize++] = Serial.read();
        
    }
    inspectRecievedSerialData(dataRecieved); // Inspect the data to determine what was recieved and what to do
  }

 
 //----------------------------------------------------------------------------------------------------- 
 //Rocket pre-launch algorithm
  do
  {
    //Make sure parachutes DO NOT activate
    digitalWrite(DROGUE_EMATCH_PIN, LOW);
    digitalWrite(MAIN_EMATCH_PIN, LOW);
    
    
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPSfromXbee(); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  }
  while((bmpData[0] < LAUNCH_THRESH) && bufferMarker >= ALTITUDE_BUFFER_SIZE); 
 
 
 
  //----------------------------------------------------------------------------------------------------- 
  //Drogue parachute deployment algorithm 
  //Read and transmit data from all sensors while vertAccel greater than APOGEE_ACCEL_THRESH
  //Send current HIGH to DROGUE parachute e-match when vertAccel less than or equal to APOGEE_ACCEL_THRESH
  do
  {
    //Make sure parachutes DO NOT activate
    digitalWrite(DROGUE_EMATCH_PIN, LOW);
    digitalWrite(MAIN_EMATCH_PIN, LOW);
    
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPSfromXbee(); //Read GPS data
    dataSDOut();  //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  }
  while(!parachuteDeploy(1));
 
 
  // Set drogue chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee
  digitalWrite(DROGUE_EMATCH_PIN, HIGH);  
  Serial1.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);
  delay(3000) //wait 3 seconds to make sure the high pulse is sent to the EMATCH
  
  //----------------------------------------------------------------------------------------------------- 
  //Main parachute deployment algorithm 
  //Read and transmit data from all sensors while bmpData[0] (rocket altitude) above MAIN_ALT_THRESH altitude
  //Send current HIGH to MAIN parachute e-match when bmpData[0] (rocket altitude) equal to or below MAIN_ALT_THRESH
  do
  {
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPS(); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  }
  while(!parachuteDeploy(2));
  
  
  //Set main chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee  
  digitalWrite(MAIN_EMATCH_PIN, HIGH);

  Serial1.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);
  delay(3000) //wait 3 seconds to make sure the high pulse is sent to the EMATCH

  //----------------------------------------------------------------------------------------------------- 
  //Continue logging all sensor data until turned off
 
  while(true)
  {
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    getGPS(); //Read GPS data
    dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
  } 
 

 
}



//CUSTOM FUNCTIONS HERE!!!
//----------------------------------------------------------------------------------------------------- 
//----------------------------------------------------------------------------------------------------- 
//----------------------------------------------------------------------------------------------------- 
boolean parachuteDeploy(int parachuteCode)
{
  
  //If parachuteCode is 1, run DROGUE deployment algorithm
  if(parachuteCode == 1)
  {
   
   //DROGUE deployment algorithm
   //   if(rocket apogee)
   //   {
   //     return true
   //   }
   
   
  }
  
  //If parachuteCode is 2, run MAIN deployment algorithm
  unsigned long altTotal = 0;
  
  if(parachuteCode == 2)
  {
    //MAIN deployment algorithm
    
    //Add up all of the values in altitudeBuffer
    for(int i=0; i<ALTITUDE_BUFFER_SIZE; i++)
    {
      altTotal += altitudeBuffer[i];
    }
    
    //If the average of these values is lower than our main parachute altitude threshold, return true
    if((altTotal/ALTITUDE_BUFFER_SIZE) < MAIN_ALT_THRESH)
    {
      return true;
    }
    //If the average is not lower than our parachute altitude threshold, return false
    else
    {
      return false;
    }  
  }
  
}





//-----------------------------------------------------------------------------------------------------
void inspectRecievedSerialData(char str[100])
{
  boolean boolPressureValue = true;
  boolean boolLaunch = true;

  for (int i = 0; i < 9; i++ ) // Determine if the data sent was a request for sea level pressure change
  {                            // or to manually deploy parachute
     if (PRESSURE_CODE[i] != str[i] )
       boolPressureValue = false;
     if (PARACHUTE_CODE[i] != str[i] )
       boolLaunch = false;
  }
  if( boolPressureValue) // Was it a pressure change request?
   {
     char intToConvert[100] = {'\0'}; // If it was then we must convert the ascii values to integers
     int i = 9; // start at 9 because everything before is the code word
     
     while ( str[i] != '\0') // read all the numbers in until we encounter \0
     {
       intToConvert[i - 9] = str[i]; 
       i++;
     }
     seaLevelPressure = atof(intToConvert); // atof converts character array into float
     //Serial.println(seaLevelPressure);
   }
   else if (boolLaunch) // Was it manual parachute deploy?
   {
      pinMode(10,OUTPUT); // Here we will have to check if the rocket is in the correct stage to allow this
      digitalWrite(10,HIGH);
   }
  
}


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







void dataSDOut()
{
  
  //Send all 9-DOF data, 9 channels
  for(int i = 0; i<7; i++)
  {
  Serial1.print(dofData[i]);
  Serial1.print("    ");
  }
  
  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
  Serial1.println(" ");
  
  //Send all BMP180 data, 4 channels
  for(int i = 0; i<4; i++)
  {
  Serial1.print(bmpData[i]);
  Serial1.print("    ");
  }
  
  
  Serial1.println(" ");
  
  //Send all GPS data, 10 channels
  for(int i =0; i<10; i++)
  {
  Serial1.print(gpsData[i]);
  Serial1.print("    ");
  }
  

  Serial1.println(" ");
  
}




void dataXbeeOut()
{
//  
//  //Send all 9-DOF data, 9 channels
  for(int i = 0; i<7; i++)
  {
    Serial2.print(dofData[i]);
    Serial2.print("    ");
  }
  
//  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
//  Serial2.println(" ");
//  
//  //Send all BMP180 data, 4 channels
  Serial2.print("BMP:ONLINE");
  for(int i = 0; i<4; i++)
  {
    Serial2.print(bmpData[i]);
    Serial2.print(":");
  }
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
}


void getGPS()
{
  if (! usingInterrupt) 
  {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
  }

  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) 
  {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  
  // approximately every 2 seconds or so, print out the current stats
  
  if (millis() - timer > 2000) 
  { 

  timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    if (GPS.fix) 
    {

      gpsData[0] = GPS.hour;
      gpsData[1] = GPS.minute;
      gpsData[2] = GPS.seconds;
      gpsData[3] = GPS.milliseconds;
      gpsData[4] = GPS.fix;  //Boolean 1 if there is a satellite fix, 0 if there isn't
      gpsData[5] = GPS.latitude;
      gpsData[6] = GPS.longitude;
      gpsData[7] = GPS.speed;  //Measured in knots
      gpsData[8] = GPS.altitude;  //Measured in centimeters
      gpsData[9] = timer;
      
      
      
    }
    
    else
    {
      Serial.println("NO FIX"); 
    }
    
  }

}

void setupGPS()
{
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //Time/location data only
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  //Fix data and altitude 
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  useInterrupt(true);
  delay(1000);
}



void useInterrupt(boolean v) 
{
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}








void getBMP()
{
 // Get a new sensor event 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressure in hPa */
  //  Serial.print(event.pressure);
     
    /* First we get the current temperature from the BMP085 */
    float temperature;
    bmp.getTemperature(&temperature);
  //  Serial.print(temperature);
    

    /* Then convert the atmospheric pressure, SLP and temp to altitude    */
    /* Update this next line with the current SLP for better results      */
    seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
 //   Serial.print("Altitude:    "); 
   // Serial.print(bmp.pressureToAltitude(seaLevelPressure,event.pressure,temperature)); 
    bmpData[0] = bmp.pressureToAltitude(seaLevelPressure,event.pressure,temperature);
    bmpData[1] = temperature;
    bmpData[2] = event.pressure;
    bmpData[3] = timer;
    
    altitudeBuffer[bufferMarker%ALTITUDE_BUFFER_SIZE] = bmpData[0];
    bufferMarker++;
    
  }
  
  else
  {
    Serial1.print("BMP180 Pressure Sensor error    ");
    Serial1.println(timer);
    Serial2.print("BMP180 Pressure Sensor error    ");
    Serial2.println(timer);
  }
 
}




void getDOF()
{
  /* Get a new sensor event */ 
  sensors_event_t event;
  
  accel.getEvent(&event); //m/s^2
  dofData[0] = event.acceleration.x;
  dofData[1] = event.acceleration.y;
  dofData[2] = event.acceleration.z;
  
  mag.getEvent(&event); //uT
  dofData[0] = event.magnetic.x;
  dofData[0] = event.magnetic.x;
  dofData[0] = event.magnetic.x; 

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  
  // Green River, UT: 10* 50' E, +/-? 0.1745 radians
  // Montreal, QC: -14* 43' W, 0.2444 radians
  float declinationAngle = 0.1745;
  heading += declinationAngle;  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
  
  dofData[7] = headingDegrees;
}




//-----------------------------------------------------------------------------------------------------
//9 DOF Setup Function

void setupDOF()
{
 Serial2.begin(9600);
 
 /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial2.println("Oops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
 
 if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Oops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
 
 //Code here 
  
}



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

Xbee Pro 900 RX --> Mega TX2 (pin 16)
Xbee Pro 900 TX --> Mega RX2 (pin 17)

Xbee Series 2 Wire RX --> Mega TX3 (pin 14)
Xbee Series 2 Wire TX --> Mega RX3 (pin 15)

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

//Global variables and constants

//This constant determines how many pressure values are averaged for each call of the "parachuteDeploy" function
const int ALTITUDE_BUFFER_SIZE = 10;

float dofData[9] = {0,0,0,0,0,0,0}; //9 channels (accel xyz, mag xyz, heading, 2 unused)
String gpsData[8] = {0,0,0,0,0,0,0,0}; // CHANGE to suit number of required data fields 
float bmpData[4] = {0,0,0,0};  // Altitude  Temperature  Pressure
float altitudeBuffer[ALTITUDE_BUFFER_SIZE]; //Variable to hold consecutive altitude readings for averaging
unsigned long bufferMarker = 0;
float vertAccel = 0;

<<<<<<< HEAD
String rocketStage = "Pre-Launch Mode";

boolean dofStatus = true; // Used to determine if sensor is online or offline
boolean gpsStatus = true;
boolean bmpStatus = true;
=======
String rocketStage = "Launch Mode";

boolean dofStatus = true; // Used to determine if sensor is online or offline
boolean gpsStatus = true;
boolean bmpStatuus = true;
>>>>>>> FETCH_HEAD

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
  
  Serial.begin(9600);
//-----------------------------------------------------------------------------------------------------  
//SERIAL PORT INITIALIZATIONS 
  
//Xbee ("Serial2") and SD Logger ("Serial1") communicate at 9600 baud rate, while the GPS ("Serial") uses 115200
  Serial1.begin(9600); //SD Logger
  Serial2.begin(9600); //Xbee  
  Serial3.begin(9600);

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

void split(char inputArray[], char delimiter, float outputArray[])
{
	string tempVal = "";
	string tempArray[10];
	int count = 0;

	for(int i = 0; i <= strlen(inputArray); i++)
	{
		if (inputArray[i] != delimiter) // not delimiter, so append to string
		{
			tempVal += inputArray[i];
      if(i == strlen(inputArray)) // last element, so no delimiter, need this special case
        tempArray[count] = tempVal;
		}
		else
		{
			tempArray[count++] = tempVal;	
			tempVal = ""; //clear val for next
		}
	}

	float floatVal;
	int periodPosition;
	float divisionFactor;

	for(int i = 0; !tempArray[i].empty(); i++)
	{
    floatVal = 0;
    divisionFactor = 10; //for each value, we divide by an increasing order of 10 so that we can fix it once we have the proper decimal point position
		periodPosition = tempArray[i].length() - 1; //this allows us to multiply the final float val to the proper value (e.g. 1.2345 becomes 123.45)

		for(int j = 0; j < (tempArray[i].length()); j++)
		{
			if(tempArray[i].at(j) == '.')
			{
				periodPosition = j - 1;
			}
			if(tempArray[i].at(j) >= 48 && tempArray[i].at(j) <= 57)
			{
				divisionFactor /= 10;

				floatVal += (float)(tempArray[i].at(j) - 48) * divisionFactor;
			}	
		}

		for(int k = 0; k < periodPosition; k++)
			floatVal *= 10;

			outputArray[i] = floatVal;
	}
}

void loop()
{
<<<<<<< HEAD
 //Serial.print("before serial 3");
 delay(500); // intial delay for all electronics
=======
  
 delay(500); // intial delay for all electronics
 
 if (Serial.available() > 0 ) // Check for incoming serial data
  {
    char dataRecieved[100] = {'\0'};
    int arraySize = 0;
    while (Serial.available() > 0) { // While we have serial data read it into an array of characters
        dataRecieved[arraySize++] = Serial.read();
        
    }
    if(inspectRecievedSerialData(dataRecieved)) // Inspect the data to determine what was recieved and what to do
	{
		split(dataReceived, ':', gpsData);
	}
  }
>>>>>>> FETCH_HEAD

 //----------------------------------------------------------------------------------------------------- 
 //Rocket pre-launch algorithm
  while((avgAltitude() < LAUNCH_THRESH) || bufferMarker <= ALTITUDE_BUFFER_SIZE)
  {
    //Make sure parachutes DO NOT activate
    digitalWrite(DROGUE_EMATCH_PIN, LOW);
    digitalWrite(MAIN_EMATCH_PIN, LOW);
    
    Serial.print("pre launch");
    analogWrite(2, 168);
    //digitalWrite(24, HIGH);
    delay(10);
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    //dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
    checkSerial();
  }
   
  rocketStage = "Apogee Detection Mode";
 
 
  //----------------------------------------------------------------------------------------------------- 
  //Drogue parachute deployment algorithm 
  //Read and transmit data from all sensors while vertAccel greater than APOGEE_ACCEL_THRESH
  //Send current HIGH to DROGUE parachute e-match when vertAccel less than or equal to APOGEE_ACCEL_THRESH
  while(!parachuteDeploy(1))
  {
    //Make sure parachutes DO NOT activate
    digitalWrite(DROGUE_EMATCH_PIN, LOW);
    digitalWrite(MAIN_EMATCH_PIN, LOW);
    analogWrite(2,168);
    //digitalWrite(24, HIGH);
    delay(10);
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    //dataSDOut();  //Transmit data to SD logger, then Xbee
    dataXbeeOut();
    //Serial.print("Drogue ");
    checkSerial();
  }
  
 
 
  // Set drogue chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee
  digitalWrite(DROGUE_EMATCH_PIN, HIGH);  
  Serial1.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("DROGUE CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);
  delay(500); //wait 3 seconds to make sure the high pulse is sent to the EMATCH
  
  rocketStage = "Main Para Detect Mode";
  
  //----------------------------------------------------------------------------------------------------- 
  //Main parachute deployment algorithm 
  //Read and transmit data from all sensors while bmpData[0] (rocket altitude) above MAIN_ALT_THRESH altitude
  //Send current HIGH to MAIN parachute e-match when bmpData[0] (rocket altitude) equal to or below MAIN_ALT_THRESH
  while(!parachuteDeploy(2))
  {
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    //getGPS(); //Read GPS data
    //dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
    
    checkSerial();
    Serial.println("main para");
  }
  
  
  
  //Set main chute pin HIGH, output message indicating signal sent and time to SD logger and Xbee  
  digitalWrite(MAIN_EMATCH_PIN, HIGH);

  Serial1.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial1.println(timer);
  Serial2.print("MAIN CHUTE DEPLOYMENT SIGNAL SENT    ");
  Serial2.println(timer);
  delay(3000); //wait 3 seconds to make sure the high pulse is sent to the EMATCH

  rocketStage = "Passive Descent Mode";

  //----------------------------------------------------------------------------------------------------- 
  //Continue logging all sensor data until turned off
 
  while(true)
  {
    delay(400);
    getDOF(); //Read 9-DOF data
    getBMP(); //Read BMP180 data
    //getGPS(); //Read GPS data
    //dataSDOut(); //Transmit data to SD logger, then Xbee
    dataXbeeOut();
    Serial.print("loop");
    checkSerial();
  } 
 

 
}



//CUSTOM FUNCTIONS HERE!!!
//----------------------------------------------------------------------------------------------------- 
//----------------------------------------------------------------------------------------------------- 
float avgAltitude()
{
  unsigned long altTotal = 0;
  
  
  //Add up all of the values in altitudeBuffer
  for(int i=0; i<ALTITUDE_BUFFER_SIZE; i++)
  {
    altTotal += altitudeBuffer[i];
  }
    
    return (altTotal/ALTITUDE_BUFFER_SIZE)
}



//----------------------------------------------------------------------------------------------------- 
bool parachuteDeploy(int parachuteCode)
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
  if(parachuteCode == 2)
  {
    //MAIN deployment algorithm 
    //If the average altitude is lower than our main parachute altitude threshold, return true
    //float currentAlt = avgAltitude(); //Unnecessary? Can avgAltitude() be called in the "if"?
    if(avgAltitude() < MAIN_ALT_THRESH)
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
bool inspectRecievedSerialData(char str[100])
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
   
   return !(boolPressureValue && boolLaunch);  
}


//----------------------------------------------------------------------------------------------------- 

void checkSerial()
{
if (Serial3.available() > 0 ) // Check for incoming serial data
  {
    Serial.println("in serial3");
    char dataRecieved[100] = {'\0'};
    int arraySize = 0;
    while (Serial3.available() > 0) 
    { // While we have serial data read it into an array of characters
        dataRecieved[arraySize++] = Serial3.read();
    }
    
    if(inspectRecievedSerialData(dataRecieved)) // Inspect the data to determine what was recieved and what to do
    {
      split(dataRecieved, ':', gpsData);
    }   
    
    for(int i=0;i<arraySize;i++)
    {
      Serial.println(dataRecieved[i]);
    }
    
  }
}

<<<<<<< HEAD
//----------------------------------------------------------------------------------------------------- 
=======
void dataXbeeOut()
{
  
  String parseString;
  if ( dofStatus )
  {
    parseString = "DOF:ONLINE";
    char dtostrfbuffer[15];
    for(int i = 0; i<7; i++) 
    {
      parseString += ":";
      parseString += dtostrf(dofData[i],8,2, dtostrfbuffer);
    }
  }
    else
      parseString = "DOF:OFFLINE";
  Serial2.println(parseString);
  
  if ( bmpStatus )
  {
    parseString = "BMP:ONLINE";
    for(int i = 0; i<4; i++)
    {
      parseString += ":";
      parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else
    parseString = "BMP:OFFLINE";
  Serial2.println(parseString);

  if ( gpsStatus )
  {
    parseString = "GPS:ONLINE:";
    for(int i =0; i<10; i++)
    {
        parseString += ":";
        parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else 
    parseString = "GPS:OFFLINE
   Serial2.println(parseString);
   
   Serial2.println( "RocketStatus:" + rocketStage );
  
}
>>>>>>> FETCH_HEAD

void dataXbeeOut()
{
  
  String parseString;
  char dtostrfbuffer[15];
  
  if (dofStatus)
  {
    parseString = "DOF:ONLINE";
    
    for(int i = 0; i<7; i++) 
    {
      parseString += ":";
      parseString += dtostrf(dofData[i],8,2, dtostrfbuffer);
    }
  }
  else
  { 
    parseString = "DOF:OFFLINE";
  }
  Serial.println(parseString);
  Serial2.println(parseString);
  
  if (bmpStatus)
  {
    parseString = "BMP:ONLINE";
    for(int i = 0; i<4; i++)
    {
      parseString += ":";
      parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else
  {
    parseString = "BMP:OFFLINE";
  }
  Serial.println(parseString);
  Serial2.println(parseString);

  if (gpsStatus)
  {
    parseString = "GPS:ONLINE:";
    for(int i =0; i<8; i++)
    {
        parseString += ":";
        parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else
  { 
    parseString = "GPS:OFFLINE";
  }
   Serial.println(parseString);
   Serial2.println(parseString);
   
   Serial2.println( "RocketStatus:" + rocketStage );
  
}

//----------------------------------------------------------------------------------------------------- 


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
  for(int i =0; i<8; i++)
  {
    Serial1.print(gpsData[i]);
    Serial1.print("    ");
  }
  

  Serial1.println(" ");
  
}


//----------------------------------------------------------------------------------------------------- 


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


//-----------------------------------------------------------------------------------------------------

void split(char inputArray[], char delimiter, String outputArray[])
{
	String tempVal = "";
	//String tempArray[10];
	int count = 0;

        //add no fix

	for(int i = 0; i <= strlen(inputArray); i++)
	{
		if (inputArray[i] != delimiter) // not delimiter, so append to string
		{
			tempVal += inputArray[i];
                        if(i == strlen(inputArray)) // last element, so no delimiter, need this special case
                          outputArray[count] = tempVal;
		}
		else
		{
			outputArray[count++] = tempVal;	
			tempVal = ""; //clear val for next
		}
	}

	//float floatVal;
	//int periodPosition;
	//float divisionFactor;



//	for(int i = 0; !tempArray[i].empty(); i++)
//	{
//    floatVal = 0;
//    divisionFactor = 10; //for each value, we divide by an increasing order of 10 so that we can fix it once we have the proper decimal point position
//		periodPosition = tempArray[i].length() - 1; //this allows us to multiply the final float val to the proper value (e.g. 1.2345 becomes 123.45)
//
//		for(int j = 0; j < (tempArray[i].length()); j++)
//		{
//			if(tempArray[i].at(j) == '.')
//			{
//				periodPosition = j - 1;
//			}
//			if(tempArray[i].at(j) >= 48 && tempArray[i].at(j) <= 57)
//			{
//				divisionFactor /= 10;
//
//				floatVal += (float)(tempArray[i].at(j) - 48) * divisionFactor;
//			}	
//		}
//
//		for(int k = 0; k < periodPosition; k++)
//			floatVal *= 10;
//
//			outputArray[i] = floatVal;
//	}
}




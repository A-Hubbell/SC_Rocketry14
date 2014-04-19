//This is a tab for all of the code to fetch data from the rocket sensors (9dof/bmp180/GPS)

#include "Arduino.h"
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Wire.h>


boolean usingInterrupt = false;   //Manual interrupt to call data
void useInterrupt(boolean);

Adafruit_GPS GPS(&Serial);
#define GPSECHO  false   //Echo to serial port

uint32_t timer = millis();

Adafruit_BMP085_U bmp = Adafruit_BMP085_U(10085);

//-----------------------------------------------------------------------------------------------------
//---------------------------------------Adafruit Ultimate GPS-----------------------------------------
//-----------------------------------------------------------------------------------------------------
//Fetch GPS Data Funtion

void getGPS(float gpsLocalData[])
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

      gpsLocalData[0] = GPS.hour;
      gpsLocalData[1] = GPS.minute;
      gpsLocalData[2] = GPS.seconds;
      gpsLocalData[3] = GPS.milliseconds;
      gpsLocalData[4] = GPS.fix;  //Boolean 1 if there is a satellite fix, 0 if there isn't
      gpsLocalData[5] = GPS.latitude;
      gpsLocalData[6] = GPS.longitude;
      gpsLocalData[7] = GPS.speed;  //Measured in knots
      gpsLocalData[8] = GPS.altitude;  //Measured in centimeters
      gpsLocalData[9] = timer;
      
      
      
    }
    
    else
    {
      Serial.println("NO FIX"); 
    }
    
  }

}

//-----------------------------------------------------------------------------------------------------
//Setup GPS Function

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

//uint32_t timer = millis();





//-----------------------------------------------------------------------------------------------------
//-----------------------------------BMP180 Pressure Sensor--------------------------------------------
//-----------------------------------------------------------------------------------------------------
//Fetch BMP180 Data Function


void getBMP(float bmpLocalData[])
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
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
 //   Serial.print("Altitude:    "); 
   // Serial.print(bmp.pressureToAltitude(seaLevelPressure,event.pressure,temperature)); 
    bmpLocalData[0] = bmp.pressureToAltitude(seaLevelPressure,event.pressure,temperature);
    bmpLocalData[1] = temperature;
    bmpLocalData[2] = event.pressure;
    bmpLocalData[3] = timer;
}
  
  else
  {
    Serial1.print("BMP180 Pressure Sensor error    ");
    Serial1.println(timer);
    Serial2.print("BMP180 Pressure Sensor error    ");
    Serial2.println(timer);
  }
 
}


//-----------------------------------------------------------------------------------------------------
//BMP180 Setup Function


//

void setupBMP180()
{
  
 //Anything to setup? 
  
}



//-----------------------------------------------------------------------------------------------------
//--------------------------------------9 DOF Sensor Stick---------------------------------------------
//-----------------------------------------------------------------------------------------------------
//Fetch 9DOF Data Function

void getDOF(float dofLocalData[])
{
  
 //Code here 
  
}




//-----------------------------------------------------------------------------------------------------
//9 DOF Setup Function

void setupDOF()
{
  
 //Code here 
  
}

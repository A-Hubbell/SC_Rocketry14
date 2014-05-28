/*

TESTED AS OF MAY 28, 2014. GPS got a fix, relayed the info to the Xbee and was picked up on a separate Xbee.
GPS Coordinates were off, but it was a cloudy day. GPS should be tested again.


This is the code for the Arduino Mega in the nose cone of the rocket. It receives data from the GPS via "Serial1" and sends it to
via the nose cone Xbee ("Serial2") to the Xbee in the controls bay. 


Connect Adafruit Ultimate GPS TX --> Mega RX1
Connect Adafruit Ultimate GPS RX --> Mega TX1

Connect Xbee DOUT (TX) --> Mega RX2
Connect Xbee DIN (RX)  --> Mega TX2



*/

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

#define GPSECHO true

float gpsData[8] = {0,0,0,0,0,0,0,0}; // CHANGE to suit number of required data fields 


boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy


Adafruit_GPS GPS(&Serial1);

void setup() 
{
  Serial.begin(115200); //GPS serial port
  Serial2.begin(9600); //Xbee serial port
  
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //Time/location data only
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  //Fix data and altitude 
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  useInterrupt(true);
  delay(1000);\
  
  
}

SIGNAL(TIMER0_COMPA_vect) 
{
  char c = GPS.read();
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

uint32_t timer = millis();

void loop() 
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
      //gpsData[4] = GPS.fix;  //Boolean 1 if there is a satellite fix, 0 if there isn't
      gpsData[4] = GPS.latitude;
      gpsData[5] = GPS.longitude;
      //gpsData[6] = GPS.speed;  //Measured in knots
      gpsData[6] = GPS.altitude;  //Measured in centimeters
      gpsData[7] = timer;
      
      //for debugging purposes only
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      
  Serial2.print("GPS:ONLINE");
  for(int i =0; i<8; i++)
  {
    Serial2.print(gpsData[i]);
    Serial2.print("    ");
  }
  
  Serial2.println(" ");
      
      
    }
    
//    else
//    {
//      Serial.println("NO FIX"); 
//    }
    
  }
  
}



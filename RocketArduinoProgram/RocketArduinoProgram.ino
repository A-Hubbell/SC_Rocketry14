#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <stdlib.h> 
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;

void inspectData(char str[100])
{
  boolean boolPressureValue = true;
  char pressure[10] = "pressure:";
  boolean boolLaunch = true;
  char launch[10] = "LaunchPar";
  char intToConvert[100] = {'\0'};
  for (int i = 0; i < 9; i++ )
  {
     if (pressure[i] != str[i] )
       boolPressureValue = false;
     if (launch[i] != str[i] )
       boolLaunch = false;
  }
  Serial.println(str);
  if( boolPressureValue)
   {
     int i = 9;
     
     while ( str[i] != '\0')
     {
       intToConvert[i - 9] = str[i];
       i++;
     }
     seaLevelPressure = atof(intToConvert);
     Serial.println(seaLevelPressure);
   }
   else if (boolLaunch)
   {
      pinMode(10,OUTPUT);
      digitalWrite(10,HIGH);
   }
  
}
void setup(void) 
{
  Serial.begin(9600); 
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("BMP:OFFLINE");
    while(1);
  }
}

void loop(void) 
{
  sensors_event_t event;
  bmp.getEvent(&event);
  if (Serial.available() > 0 )
  {
    char str[100] = {'\0'};
    int arraySize = 0;
    while (Serial.available() > 0) {
        str[arraySize++] = Serial.read();
        
    }
    inspectData(str);
  }
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("BMP:ONLINE:");
    Serial.print(event.pressure);
    Serial.print(" hPa:");
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print(temperature);
    Serial.print("C:");
    
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure,
                                        temperature)); 
    Serial.print(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("BMP:OFFLINE");
  }
  delay(1000);
}



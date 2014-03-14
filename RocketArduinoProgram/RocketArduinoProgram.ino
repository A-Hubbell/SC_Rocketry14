#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <stdlib.h> 
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
void inspectData(char str[100])
{
  boolean boolPressureValue = true;
  char pressure[10] = "pressure:";
  char intToConvert[100] = {'\0'};
  for (int i = 0; i < 10; i++ )
  {
     if (pressure[i] != str[i] )
       boolPressureValue = false;
  }
  
  if( boolPressureValue)
   {
     int i = 10;
     while ( str[i] != '\0')
       intToConvert[i++] = str[i];
     double pressureValue = atof(intToConvert);
     Serial.print(pressureValue);
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
  Serial.print("test");
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
  /*if (event.pressure)
  {
    /* Display atmospheric pressue in hPa 
    Serial.print("BMP:ONLINE:");
    Serial.print(event.pressure);
    Serial.print(" hPa:");
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print(temperature);
    Serial.print("C:");
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure,
                                        temperature)); 
    Serial.print(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("BMP:OFFLINE");
  }*/
  delay(1000);
}



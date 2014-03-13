#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
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
  while (Serial.available() > 0) {
      byte data = Serial.read();
      String text = String(data);
      Serial.print(text); 
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

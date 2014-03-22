/* TODO:
 * compass.setScale not registering valid input
 * what is Gauss scale?
 * find proper declination angle for Green River, Utah
 * ADXL get/setRangeSetting (resolution)
 * ADXL get/setFullRangeBit
 */
#include <Wire.h>
#include <FreeSixIMU.h>
#include <FIMU_ADXL345.h>
#include <FIMU_ITG3200.h>
#include <HMC5883L.h>

float angles[3]; //yaw pitch roll
float heading;

FreeSixIMU sixDOF = FreeSixIMU();
HMC5883L compass;

//record any errors that may occur in the compass
int error = 0;

void setup()
{  
  Serial.begin(9600);
  Wire.begin();

  delay(5);
  sixDOF.init(); //init the Acc and Gyro
  delay(5);
  compass = HMC5883L(); //init HMC5883
  
  error = compass.SetScale((float)1.3); //set the scale of the compass.
  error = compass.SetMeasurementMode(Measurement_Continuous); //set the measurement mode to continuous
  
  if(error != 0) //print error messages
    Serial.println(compass.GetErrorText(error));
}

void loop()
{  
  sixDOF.getEuler(angles);
  getHeading();
  
  PrintData();  
  delay(300);
}

void getHeading()
{  
  //get raw values from the compass
  MagnetometerRaw raw = compass.ReadRawAxis();
  //get the scaled values from the compass (scaled to the configured scale)
  MagnetometerScaled scaled = compass.ReadScaledAxis();

  //calculate heading when the magnetometer is level, then correct for signs of axis.
  heading = atan2(scaled.YAxis, scaled.XAxis);  
  
  float declinationAngle = 0.0457;
  heading += declinationAngle;
  
  //correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;    
  //check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;   
  //convert radians to degrees for readability.
  heading = heading * 180/M_PI; 
}

void PrintData()
{  
  Serial.print("Euler Angles: ");
  Serial.print(angles[0]); //yaw
  Serial.write(176); //byte 176: ° character
  Serial.print("  ");  
  Serial.print(angles[1]); //pitch
  Serial.write(176);
  Serial.print("  ");
  Serial.print(angles[2]); //roll
  Serial.write(176);
  Serial.print("  ");
  Serial.print("Heading: ");
  Serial.print(heading);
  Serial.write(176);
  Serial.print("\n");  
}

